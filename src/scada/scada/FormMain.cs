using scada.Controllers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace scada
{
    public partial class FormMain : Form
    {
        private SerialController _serialController;
        public FormMain()
        {
            InitializeComponent();

            _serialController = new SerialController();

            // Inscreve nos eventos da classe
            _serialController.OnLogReceived += AtualizarLog;
            _serialController.OnConnectionChanged += AtualizarStatusConexao;
            _serialController.OnValidationComplete += MostrarResultadoValidacao;
        }

        private void FormMain_Load(object sender, EventArgs e)
        {
            cmbPortas.DataSource = _serialController.GetAvailablePorts();
        }

        private void btnConectar_Click(object sender, EventArgs e)
        {
            string porta = cmbPortas.SelectedItem?.ToString();
            if (!string.IsNullOrEmpty(porta))
            {
                _serialController.Connect(porta);
            }
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

            btnConectar.Enabled = !conectado;
            btnDesconectar.Enabled = conectado;
            cmbPortas.Enabled = !conectado;
        }

        private void btnDesconectar_Click(object sender, EventArgs e)
        {
            _serialController.Disconnect();
        }

        // Boa prática: Fechar porta ao fechar janela
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            _serialController.Dispose();
        }
    }
}
