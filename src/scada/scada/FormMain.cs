using System;
using System.Configuration;
using System.Drawing;
using System.Threading.Tasks;
using System.Windows.Forms;
using scada.Controllers;
using scada.Services;

namespace scada
{
    public partial class FormMain : Form
    {
        private MqttController _mqttController;
        private ApiService _apiService;
        private AdbController _adbController;

        private string modelo = ConfigurationManager.AppSettings["ModeloCelular"];
        private float rRef, gRef, bRef;

        public FormMain()
        {
            InitializeComponent();
            _mqttController = new MqttController();
            _apiService = new ApiService(ConfigurationManager.AppSettings["ApiUrl"]);
            _adbController = new AdbController();

            _mqttController.OnLogReceived += AtualizarLog;
            _mqttController.OnValidationComplete += MostrarResultadoValidacao;
        }

        private void FormMain_Load(object sender, EventArgs e) => _ = LoadingAsync();

        private async Task LoadingAsync()
        {
            var config = await _apiService.GetConfiguracaoModelo(modelo);
            if (config != null)
            {
                rRef = config.R; gRef = config.G; bRef = config.B;
                lblModelo.Text = $"Modelo: {modelo}";
                lblCorAlvo.Text = $"Alvo: {config.Nome}";
                AtualizarLog($"Configuração {config.Nome} carregada da API.");
            }
            _mqttController.Connect();
        }

        private async void MostrarResultadoValidacao(int rLido, int gLido, int bLido)
        {
            if (InvokeRequired) { Invoke(new Action(() => MostrarResultadoValidacao(rLido, gLido, bLido))); return; }
            // 1. Limpeza de tela
            if(rLido.Equals(0) && gLido.Equals(0) && bLido.Equals(0))
            {
                AtualizarLog("Aguardando...");
                lblStatus.Text = "AGUARDANDO...";
                pnlResultado.BackColor = Color.LightGray;
                return;
            }

            // 2. Validação da Cor
            double dist = Math.Sqrt(Math.Pow(rLido - rRef, 2) + Math.Pow(gLido - gRef, 2) + Math.Pow(bLido - bRef, 2));
            if (dist >= 30) 
            { 
                Finalizar(false, "Cor Inválida"); 
                return; 
            }

            // 3. Captura de Serial (Só se a cor passar)
            AtualizarLog("Cor OK. Capturando Serial...");
            string serial = await _adbController.GetSerialNumber();
            if (serial.Contains("ERRO") || serial.Contains("NENHUM")) 
            { 
                Finalizar(false, $"Serial: {serial}"); 
                return; 
            }

            // 4. Salvar no MySQL via API
            bool salvo = await _apiService.SalvarValidacao(modelo, serial, true);
            if (salvo)
            {
                Finalizar(true, $"Sucesso: {serial}");
                lblValores.Text = $"R:{rLido} G:{gLido} B:{bLido} | ID:{serial}";
            }
            else
            {
                AtualizarLog("Erro ao salvar no banco.");
            }
        }

        private void Finalizar(bool aprovado, string msg)
        {
            lblStatus.Text = aprovado ? "APROVADO" : "REPROVADO";
            pnlResultado.BackColor = aprovado ? Color.Green : Color.Red;
            _mqttController.EnviarRespostaPico(aprovado);
            AtualizarLog(msg);
        }

        private void AtualizarLog(string m)
        {
            if (txtLog.InvokeRequired)
            {
                txtLog.Invoke(new Action(() => AtualizarLog(m)));
                return;
            }
            txtLog.AppendText($"{DateTime.Now:HH:mm:ss} - {m}{Environment.NewLine}");
        }
        private void FormMain_FormClosing(object sender, FormClosingEventArgs e) => _mqttController.Dispose();
    }
}