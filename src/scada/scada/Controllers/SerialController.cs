using System;
using System.IO.Ports;
using System.Diagnostics;
using System.Collections.Generic;
using scada.Models;

namespace scada.Controllers
{
    public class SerialController : IDisposable
    {
        private SerialPort _serialPort;

        public event Action<string> OnLogReceived;       // Para logs gerais
        public event Action<bool, int, int, int> OnValidationComplete; // Quando validar (Aprovado?, R, G, B)
        public event Action<bool> OnConnectionChanged;   // Conectou ou Desconectou

        public SerialController()
        {
            _serialPort = new SerialPort();
            _serialPort.DataReceived += SerialPort_DataReceived;
        }

        // Lista as portas disponíveis
        public string[] GetAvailablePorts()
        {
            return SerialPort.GetPortNames();
        }

        public void Connect(string portName, int baudRate = 115200)
        {
            if (_serialPort.IsOpen) _serialPort.Close();

            _serialPort.PortName = portName;
            _serialPort.BaudRate = baudRate;
            _serialPort.ReadTimeout = 500;
            _serialPort.WriteTimeout = 500;

            // --- CORREÇÃO PRINCIPAL AQUI ---
            // O Raspberry Pi Pico PRECISA disso para enviar dados via USB
            _serialPort.DtrEnable = true;
            _serialPort.RtsEnable = true;
            // -------------------------------

            try
            {
                _serialPort.Open();

                // Limpa qualquer lixo que tenha ficado no buffer antes de começar
                _serialPort.DiscardInBuffer();

                OnConnectionChanged?.Invoke(true);
                Log($"Conectado na porta {portName}");
            }
            catch (Exception ex)
            {
                Log($"Erro ao conectar: {ex.Message}");
            }
        }

        public void Disconnect()
        {
            if (_serialPort.IsOpen)
            {
                _serialPort.Close();
                OnConnectionChanged?.Invoke(false);
                Log("Desconectado.");
            }
        }

        private void SerialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            if (!_serialPort.IsOpen) return;

            try
            {
                // Enquanto houver dados no buffer, continue lendo
                while (_serialPort.BytesToRead > 0)
                {
                    string line = _serialPort.ReadLine();

                    if (string.IsNullOrWhiteSpace(line)) continue;

                    line = line.Trim();

                    // Se for um dado de cor, processa
                    if (line.StartsWith("DATA:"))
                    {
                        ProcessarDadosCor(line);
                    }
                    else
                    {
                        // Apenas loga outras mensagens (debug)
                        Log($"[PICO]: {line}");
                    }
                }
            }
            catch (TimeoutException)
            {
                // Timeout é normal em leitura serial, apenas ignora e espera o próximo evento
            }
            catch (Exception ex)
            {
                Log($"Erro de leitura: {ex.Message}");
            }
        }

        private readonly List<CorModel> _coresReferencia = new List<CorModel>
{
            // Preto: (166*255)/726, etc...
            new CorModel("Preto",   (166f * 255f) / 726f,   (256f * 255f) / 726f,   (242f * 255f) / 726f),
            // Branco: (2072*255)/7867...
            new CorModel("Branco",  (2072f * 255f) / 7867f, (2765f * 255f) / 7867f, (2540f * 255f) / 7867f),
            // Amarelo
            new CorModel("Amarelo", (1505f * 255f) / 4163f, (1554f * 255f) / 4163f, (813f * 255f) / 4163f),
            // Azul
            new CorModel("Azul",    (295f * 255f) / 2341f,  (663f * 255f) / 2341f,  (1262f * 255f) / 2341f)
        };

        private void ProcessarDadosCor(string rawData)
        {
            try
            {
                // Formato esperado AGORA: DATA:R,G,B,C
                string content = rawData.Substring(5);
                string[] parts = content.Split(',');

                // Precisamos de 4 valores (R, G, B, C)
                if (parts.Length >= 4)
                {
                    int r = int.Parse(parts[0]);
                    int g = int.Parse(parts[1]);
                    int b = int.Parse(parts[2]);
                    int c = int.Parse(parts[3]);

                    Log($"Lido: R={r} G={g} B={b} C={c}");

                    // === 1. LÓGICA DE IDENTIFICAÇÃO (Igual ao C) ===
                    string corDetectada = IdentificarCor(r, g, b, c);

                    Log($"-> Cor Detectada: {corDetectada}");

                    // === 2. REGRA DE VALIDAÇÃO ===
                    // Exemplo: Aprovar se for AZUL ou BRANCO (ajuste conforme sua necessidade)
                    bool aprovado = (corDetectada == "Azul");

                    // Envia resposta para o Pico
                    string resposta = aprovado ? "A" : "R";
                    _serialPort.Write(resposta);

                    // Atualiza UI (Passando a cor detectada como string extra se quiser)
                    OnValidationComplete?.Invoke(aprovado, r, g, b);
                }
            }
            catch (Exception ex)
            {
                Log($"Erro proc: {ex.Message}");
            }
        }

        private string IdentificarCor(int r, int g, int b, int c)
        {
            // Se a iluminação for muito baixa, assume preto
            if (c < 100) return "Preto";

            // Normaliza os valores RGB para a escala 0–255 usando o Clear (C)
            float rn = (r * 255.0f) / c;
            float gn = (g * 255.0f) / c;
            float bn = (b * 255.0f) / c;

            // Garante limite 255
            if (rn > 255) rn = 255;
            if (gn > 255) gn = 255;
            if (bn > 255) bn = 255;

            float menorDistancia = float.MaxValue;
            string corResultado = "Desconhecida";

            // Cálculo da Distância Euclidiana
            foreach (var corRef in _coresReferencia)
            {
                float dr = rn - corRef.R;
                float dg = gn - corRef.G;
                float db = bn - corRef.B;

                // Pitágoras 3D (d² = x² + y² + z²)
                float dist = (float)Math.Sqrt(dr * dr + dg * dg + db * db);

                if (dist < menorDistancia)
                {
                    menorDistancia = dist;
                    corResultado = corRef.Nome;
                }
            }

            return corResultado;
        }

        private void Log(string msg)
        {
            // Dispara o evento para quem estiver escutando (o Form)
            OnLogReceived?.Invoke($"{DateTime.Now:HH:mm:ss} - {msg}");
        }

        public void Dispose()
        {
            if (_serialPort != null)
            {
                if (_serialPort.IsOpen) _serialPort.Close();
                _serialPort.Dispose();
            }
        }
    }
}
