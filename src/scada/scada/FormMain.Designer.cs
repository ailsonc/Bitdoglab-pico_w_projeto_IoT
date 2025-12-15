namespace scada
{
    partial class FormMain
    {
        /// <summary>
        /// Variável de designer necessária.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Limpar os recursos que estão sendo usados.
        /// </summary>
        /// <param name="disposing">true se for necessário descartar os recursos gerenciados; caso contrário, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Código gerado pelo Windows Form Designer

        /// <summary>
        /// Método necessário para suporte ao Designer - não modifique 
        /// o conteúdo deste método com o editor de código.
        /// </summary>
        private void InitializeComponent()
        {
            this.cmbPortas = new System.Windows.Forms.ComboBox();
            this.btnConectar = new System.Windows.Forms.Button();
            this.btnDesconectar = new System.Windows.Forms.Button();
            this.txtLog = new System.Windows.Forms.TextBox();
            this.pnlResultado = new System.Windows.Forms.Panel();
            this.lblStatus = new System.Windows.Forms.Label();
            this.lblValores = new System.Windows.Forms.Label();
            this.grpConexao = new System.Windows.Forms.GroupBox();
            this.grpMonitoramento = new System.Windows.Forms.GroupBox();
            this.label1 = new System.Windows.Forms.Label(); // Label auxiliar para "Porta:"
            this.label2 = new System.Windows.Forms.Label(); // Label auxiliar para "Log:"
            this.pnlResultado.SuspendLayout();
            this.grpConexao.SuspendLayout();
            this.grpMonitoramento.SuspendLayout();
            this.SuspendLayout();
            // 
            // grpConexao
            // 
            this.grpConexao.Controls.Add(this.label1);
            this.grpConexao.Controls.Add(this.cmbPortas);
            this.grpConexao.Controls.Add(this.btnConectar);
            this.grpConexao.Controls.Add(this.btnDesconectar);
            this.grpConexao.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.grpConexao.Location = new System.Drawing.Point(12, 12);
            this.grpConexao.Name = "grpConexao";
            this.grpConexao.Size = new System.Drawing.Size(460, 80);
            this.grpConexao.TabIndex = 0;
            this.grpConexao.TabStop = false;
            this.grpConexao.Text = "Configuração de Conexão";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(15, 33);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(71, 15);
            this.label1.TabIndex = 3;
            this.label1.Text = "Porta Serial:";
            // 
            // cmbPortas
            // 
            this.cmbPortas.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbPortas.FormattingEnabled = true;
            this.cmbPortas.Location = new System.Drawing.Point(92, 30);
            this.cmbPortas.Name = "cmbPortas";
            this.cmbPortas.Size = new System.Drawing.Size(120, 23);
            this.cmbPortas.TabIndex = 0;
            // 
            // btnConectar
            // 
            this.btnConectar.BackColor = System.Drawing.Color.ForestGreen;
            this.btnConectar.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.btnConectar.ForeColor = System.Drawing.Color.White;
            this.btnConectar.Location = new System.Drawing.Point(230, 27);
            this.btnConectar.Name = "btnConectar";
            this.btnConectar.Size = new System.Drawing.Size(100, 30);
            this.btnConectar.TabIndex = 1;
            this.btnConectar.Text = "Conectar";
            this.btnConectar.UseVisualStyleBackColor = false;
            this.btnConectar.Click += new System.EventHandler(this.btnConectar_Click);
            // 
            // btnDesconectar
            // 
            this.btnDesconectar.BackColor = System.Drawing.Color.Firebrick;
            this.btnDesconectar.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.btnDesconectar.ForeColor = System.Drawing.Color.White;
            this.btnDesconectar.Location = new System.Drawing.Point(340, 27);
            this.btnDesconectar.Name = "btnDesconectar";
            this.btnDesconectar.Size = new System.Drawing.Size(100, 30);
            this.btnDesconectar.TabIndex = 2;
            this.btnDesconectar.Text = "Desconectar";
            this.btnDesconectar.UseVisualStyleBackColor = false;
            this.btnDesconectar.Click += new System.EventHandler(this.btnDesconectar_Click);
            // 
            // grpMonitoramento
            // 
            this.grpMonitoramento.Controls.Add(this.pnlResultado);
            this.grpMonitoramento.Controls.Add(this.lblValores);
            this.grpMonitoramento.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.grpMonitoramento.Location = new System.Drawing.Point(12, 100);
            this.grpMonitoramento.Name = "grpMonitoramento";
            this.grpMonitoramento.Size = new System.Drawing.Size(460, 338);
            this.grpMonitoramento.TabIndex = 1;
            this.grpMonitoramento.TabStop = false;
            this.grpMonitoramento.Text = "Validação de Cor";
            // 
            // pnlResultado
            // 
            this.pnlResultado.BackColor = System.Drawing.Color.LightGray;
            this.pnlResultado.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.pnlResultado.Controls.Add(this.lblStatus);
            this.pnlResultado.Location = new System.Drawing.Point(20, 35);
            this.pnlResultado.Name = "pnlResultado";
            this.pnlResultado.Size = new System.Drawing.Size(420, 200);
            this.pnlResultado.TabIndex = 0;
            // 
            // lblStatus
            // 
            this.lblStatus.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lblStatus.Font = new System.Drawing.Font("Segoe UI", 24F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblStatus.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
            this.lblStatus.Location = new System.Drawing.Point(0, 0);
            this.lblStatus.Name = "lblStatus";
            this.lblStatus.Size = new System.Drawing.Size(418, 198);
            this.lblStatus.TabIndex = 0;
            this.lblStatus.Text = "AGUARDANDO...";
            this.lblStatus.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // lblValores
            // 
            this.lblValores.Font = new System.Drawing.Font("Consolas", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblValores.Location = new System.Drawing.Point(20, 250);
            this.lblValores.Name = "lblValores";
            this.lblValores.Size = new System.Drawing.Size(420, 30);
            this.lblValores.TabIndex = 1;
            this.lblValores.Text = "R: 0 | G: 0 | B: 0";
            this.lblValores.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Bold);
            this.label2.Location = new System.Drawing.Point(490, 12);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(95, 15);
            this.label2.TabIndex = 4;
            this.label2.Text = "Log do Sistema:";
            // 
            // txtLog
            // 
            this.txtLog.BackColor = System.Drawing.Color.Black;
            this.txtLog.Font = new System.Drawing.Font("Consolas", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.txtLog.ForeColor = System.Drawing.Color.Lime;
            this.txtLog.Location = new System.Drawing.Point(490, 35);
            this.txtLog.Multiline = true;
            this.txtLog.Name = "txtLog";
            this.txtLog.ReadOnly = true;
            this.txtLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.txtLog.Size = new System.Drawing.Size(282, 403);
            this.txtLog.TabIndex = 2;
            // 
            // FormMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 450);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.txtLog);
            this.Controls.Add(this.grpMonitoramento);
            this.Controls.Add(this.grpConexao);
            this.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Name = "FormMain";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "SCADA - Validador de Cor";
            this.Load += new System.EventHandler(this.FormMain_Load);
            this.pnlResultado.ResumeLayout(false);
            this.grpConexao.ResumeLayout(false);
            this.grpConexao.PerformLayout();
            this.grpMonitoramento.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox cmbPortas;
        private System.Windows.Forms.Button btnConectar;
        private System.Windows.Forms.Button btnDesconectar;
        private System.Windows.Forms.TextBox txtLog;
        private System.Windows.Forms.Panel pnlResultado;
        private System.Windows.Forms.Label lblStatus;
        private System.Windows.Forms.Label lblValores;
        private System.Windows.Forms.GroupBox grpConexao;
        private System.Windows.Forms.GroupBox grpMonitoramento;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
    }
}