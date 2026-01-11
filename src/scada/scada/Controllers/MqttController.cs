using System;
using System.Text;
using System.Collections.Generic;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;
using scada.Models;
using Newtonsoft.Json; // Adicione via NuGet para processar o JSON do Pico

namespace scada.Controllers
{
    public class MqttController : IDisposable
    {
        private MqttClient _client;
        private string _brokerAddress;

        // Mantive os eventos para não quebrar seu FormMain
        public event Action<string> OnLogReceived;
        public event Action<bool, int, int, int> OnValidationComplete;
        public event Action<bool> OnConnectionChanged;

        private const string BROKER_URL = "broker.hivemq.com";

        // Tópicos (Devem ser iguais aos do ENV.h do Pico)
        private const string TOPIC_SUB_PICO = "pico/projetoIOT/color_data"; 
        private const string TOPIC_PUB_PICO = "pico/projetoIOT/response";

        private readonly List<CorModel> _coresReferencia = new List<CorModel>
        {
            new CorModel("Preto", 58f, 90f, 85f),
            new CorModel("Branco", 67f, 90f, 82f),
            new CorModel("Amarelo", 92f, 95f, 50f),
            new CorModel("Azul", 32f, 72f, 137f),
            new CorModel("Verde", 40f, 120f, 50f),
            new CorModel("Vermelho", 130f, 50f, 50f)
        };

        public void Connect()
        {
            System.Threading.Tasks.Task.Run(() => {
                try
                {
                    // Força a resolução do DNS para obter o endereço IPv4
                    var hostEntry = System.Net.Dns.GetHostEntry(BROKER_URL);
                    var ipv4Address = Array.Find(hostEntry.AddressList,
                        a => a.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork);

                    if (ipv4Address == null) throw new Exception("Não foi possível encontrar o endereço IPv4 do Broker.");

                    // Inicializa o cliente com o IP fixo IPv4 e porta 1883
                    _client = new MqttClient(ipv4Address.ToString(), 1883, false, null, null, MqttSslProtocols.None);

                    string clientId = "SCADA_" + Guid.NewGuid().ToString().Substring(0, 5);
                    _client.Connect(clientId);

                    if (_client.IsConnected)
                    {
                        _client.Subscribe(new string[] { TOPIC_SUB_PICO }, new byte[] { 1 });
                        _client.MqttMsgPublishReceived += Client_MqttMsgPublishReceived;

                        OnConnectionChanged?.Invoke(true);
                        Log($"Conectado ao HiveMQ via IPv4 ({ipv4Address})");
                    }
                }
                catch (Exception ex)
                {
                    Log($"Erro Crítico: {ex.Message}");
                    if (ex.InnerException != null) Log($"Detalhe: {ex.InnerException.Message}");
                    OnConnectionChanged?.Invoke(false);
                }
            });
        }

        private void Client_MqttMsgPublishReceived(object sender, MqttMsgPublishEventArgs e)
        {
            string message = Encoding.UTF8.GetString(e.Message);
            Log($"Mensagem recebida: {message}");

            // O Pico envia JSON: {"r":10,"g":20,"b":30,"c":100}
            try
            {
                var data = JsonConvert.DeserializeObject<dynamic>(message);
                ProcessarDadosCor((int)data.r, (int)data.g, (int)data.b, (int)data.c);
            }
            catch (Exception ex)
            {
                Log("Erro ao processar JSON: " + ex.Message);
            }
        }

        private void ProcessarDadosCor(int r, int g, int b, int c)
        {
            string corDetectada = IdentificarCor(r, g, b, c);
            Log($"-> Cor Detectada: {corDetectada}");

            // REGRA: Aprovar se for AZUL
            bool aprovado = (corDetectada == "Azul");

            // Publica a resposta para o Pico
            string resposta = aprovado ? "APPROVED" : "REJECTED";
            _client.Publish(TOPIC_PUB_PICO, Encoding.UTF8.GetBytes(resposta));

            OnValidationComplete?.Invoke(aprovado, r, g, b);
        }

        private string IdentificarCor(int r, int g, int b, int c)
        {
            if (c < 100) return "Preto";
            float rn = Math.Min((r * 255.0f) / c, 255);
            float gn = Math.Min((g * 255.0f) / c, 255);
            float bn = Math.Min((b * 255.0f) / c, 255);

            float menorDistancia = float.MaxValue;
            string corResultado = "Desconhecida";

            foreach (var corRef in _coresReferencia)
            {
                float dist = (float)Math.Sqrt(Math.Pow(rn - corRef.R, 2) + Math.Pow(gn - corRef.G, 2) + Math.Pow(bn - corRef.B, 2));
                if (dist < menorDistancia)
                {
                    menorDistancia = dist;
                    corResultado = corRef.Nome;
                }
            }
            return corResultado;
        }

        private void Log(string msg) => OnLogReceived?.Invoke($"{DateTime.Now:HH:mm:ss} - {msg}");

        public void Disconnect()
        {
            if (_client != null && _client.IsConnected)
            {
                _client.Disconnect();
                OnConnectionChanged?.Invoke(false);
                Log("Desconectado do Broker.");
            }
        }

        public void Dispose() => Disconnect();
    }
}