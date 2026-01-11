using System;
using System.Text;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;
using Newtonsoft.Json;

namespace scada.Controllers
{
    public class MqttController : IDisposable
    {
        private MqttClient _client;
        public event Action<string> OnLogReceived;
        public event Action<int, int, int> OnValidationComplete;
        public event Action<bool> OnConnectionChanged;

        private const string BROKER_URL = "broker.hivemq.com";
        private const string TOPIC_SUB_PICO = "pico/projetoIOT/color_data";
        private const string TOPIC_PUB_PICO = "pico/projetoIOT/response";

        public void Connect()
        {
            System.Threading.Tasks.Task.Run(() => {
                try
                {
                    var hostEntry = System.Net.Dns.GetHostEntry(BROKER_URL);
                    var ipv4Address = Array.Find(hostEntry.AddressList,
                        a => a.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork);

                    if (ipv4Address == null) throw new Exception("IP não encontrado.");

                    _client = new MqttClient(ipv4Address.ToString(), 1883, false, null, null, MqttSslProtocols.None);
                    _client.Connect("SCADA_" + Guid.NewGuid().ToString().Substring(0, 5));

                    if (_client.IsConnected)
                    {
                        _client.Subscribe(new string[] { TOPIC_SUB_PICO }, new byte[] { 1 });
                        _client.MqttMsgPublishReceived += Client_MqttMsgPublishReceived;
                        OnConnectionChanged?.Invoke(true);
                        Log($"Conectado ao HiveMQ.");
                    }
                }
                catch (Exception ex)
                {
                    Log($"Erro: {ex.Message}");
                    OnConnectionChanged?.Invoke(false);
                }
            });
        }

        private void Client_MqttMsgPublishReceived(object sender, MqttMsgPublishEventArgs e)
        {
            try
            {
                string message = Encoding.UTF8.GetString(e.Message);
                var data = JsonConvert.DeserializeObject<dynamic>(message);

                OnValidationComplete?.Invoke((int)data.r, (int)data.g, (int)data.b);
            }
            catch (Exception ex) { Log("Erro JSON: " + ex.Message); }
        }

        // Método para o FormMain enviar a resposta após validar contra r_ref, g_ref, b_ref
        public void EnviarRespostaPico(bool aprovado)
        {
            if (_client != null && _client.IsConnected)
            {
                string payload = aprovado ? "APPROVED" : "REJECTED";
                _client.Publish(TOPIC_PUB_PICO, Encoding.UTF8.GetBytes(payload));
            }
        }

        private void Log(string msg) => OnLogReceived?.Invoke($"{DateTime.Now:HH:mm:ss} - {msg}");
        public void Disconnect() { if (_client != null && _client.IsConnected) _client.Disconnect(); }
        public void Dispose() => Disconnect();
    }
}