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
            this.txtLog = new System.Windows.Forms.TextBox();
            this.pnlResultado = new System.Windows.Forms.Panel();
            this.lblStatus = new System.Windows.Forms.Label();
            this.lblValores = new System.Windows.Forms.Label();
            this.grpMonitoramento = new System.Windows.Forms.GroupBox();
            this.label2 = new System.Windows.Forms.Label();
            this.grpModelo = new System.Windows.Forms.GroupBox();
            this.lblCorAlvo = new System.Windows.Forms.Label();
            this.lblModelo = new System.Windows.Forms.Label();
            this.pnlResultado.SuspendLayout();
            this.grpMonitoramento.SuspendLayout();
            this.grpModelo.SuspendLayout();
            this.SuspendLayout();
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
            // grpMonitoramento
            // 
            this.grpMonitoramento.Controls.Add(this.pnlResultado);
            this.grpMonitoramento.Controls.Add(this.lblValores);
            this.grpMonitoramento.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.grpMonitoramento.Location = new System.Drawing.Point(12, 135);
            this.grpMonitoramento.Name = "grpMonitoramento";
            this.grpMonitoramento.Size = new System.Drawing.Size(460, 303);
            this.grpMonitoramento.TabIndex = 1;
            this.grpMonitoramento.TabStop = false;
            this.grpMonitoramento.Text = "Validação de Cor";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Bold);
            this.label2.Location = new System.Drawing.Point(490, 12);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(120, 20);
            this.label2.TabIndex = 4;
            this.label2.Text = "Log do Sistema:";
            // 
            // grpModelo
            // 
            this.grpModelo.Controls.Add(this.lblCorAlvo);
            this.grpModelo.Controls.Add(this.lblModelo);
            this.grpModelo.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.grpModelo.Location = new System.Drawing.Point(12, 12);
            this.grpModelo.Name = "grpModelo";
            this.grpModelo.Size = new System.Drawing.Size(460, 117);
            this.grpModelo.TabIndex = 2;
            this.grpModelo.TabStop = false;
            this.grpModelo.Text = "Configuração";
            // 
            // lblCorAlvo
            // 
            this.lblCorAlvo.Font = new System.Drawing.Font("Consolas", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblCorAlvo.Location = new System.Drawing.Point(16, 69);
            this.lblCorAlvo.Name = "lblCorAlvo";
            this.lblCorAlvo.Size = new System.Drawing.Size(420, 30);
            this.lblCorAlvo.TabIndex = 2;
            this.lblCorAlvo.Text = "Cor Alvo: ---";
            // 
            // lblModelo
            // 
            this.lblModelo.Font = new System.Drawing.Font("Consolas", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblModelo.Location = new System.Drawing.Point(16, 36);
            this.lblModelo.Name = "lblModelo";
            this.lblModelo.Size = new System.Drawing.Size(420, 30);
            this.lblModelo.TabIndex = 1;
            this.lblModelo.Text = "Modelo: ---";
            // 
            // FormMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 450);
            this.Controls.Add(this.grpModelo);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.txtLog);
            this.Controls.Add(this.grpMonitoramento);
            this.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Name = "FormMain";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "SCADA - Validador de Cor";
            this.Load += new System.EventHandler(this.FormMain_Load);
            this.pnlResultado.ResumeLayout(false);
            this.grpMonitoramento.ResumeLayout(false);
            this.grpModelo.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.TextBox txtLog;
        private System.Windows.Forms.Panel pnlResultado;
        private System.Windows.Forms.Label lblStatus;
        private System.Windows.Forms.Label lblValores;
        private System.Windows.Forms.GroupBox grpMonitoramento;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.GroupBox grpModelo;
        private System.Windows.Forms.Label lblModelo;
        private System.Windows.Forms.Label lblCorAlvo;
    }
}