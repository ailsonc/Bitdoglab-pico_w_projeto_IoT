using System;
using System.Drawing;
using System.Windows.Forms;
using scada.Controllers;

namespace scada
{
    public partial class FormMain : Form
    {
        private MqttController _mqttController;
        public FormMain()
        {
            InitializeComponent();

            _mqttController = new MqttController();

            // Inscreve nos eventos da classe
            _mqttController.OnLogReceived += AtualizarLog;
            _mqttController.OnConnectionChanged += AtualizarStatusConexao;
            _mqttController.OnValidationComplete += MostrarResultadoValidacao;
        }

        private void FormMain_Load(object sender, EventArgs e)
        {
            AtualizarLog("Iniciando conexão automática...");
            _mqttController.Connect();
        }

        // --- MÉTODOS DE CALLBACK (Vêm de outra Thread) ---

        private void AtualizarLog(string mensagem)
        {
            // Garante que rode na Thread da UI
            if (InvokeRequired)
            {
                Invoke(new Action(() => AtualizarLog(mensagem)));
                return;
            }

            txtLog.AppendText(mensagem + Environment.NewLine);
        }

        private void MostrarResultadoValidacao(bool aprovado, int r, int g, int b)
        {
            if (InvokeRequired)
            {
                Invoke(new Action(() => MostrarResultadoValidacao(aprovado, r, g, b)));
                return;
            }

            // Atualiza UI visualmente
            lblStatus.Text = aprovado ? "APROVADO" : "REPROVADO";
            lblStatus.ForeColor = Color.White;
            pnlResultado.BackColor = aprovado ? Color.Green : Color.Red;

            // Mostra os valores lidos
            lblValores.Text = $"R: {r} | G: {g} | B: {b}";
        }

        private void AtualizarStatusConexao(bool conectado)
        {
            if (InvokeRequired)
            {
                Invoke(new Action(() => AtualizarStatusConexao(conectado)));
                return;
            }
        }

        private void btnDesconectar_Click(object sender, EventArgs e)
        {
            _mqttController.Disconnect();
        }

        // Boa prática: Fechar porta ao fechar janela
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            _mqttController.Dispose();
        }
    }
}
