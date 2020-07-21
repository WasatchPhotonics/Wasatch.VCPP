namespace WasatchVCPPNet
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea1 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Legend legend1 = new System.Windows.Forms.DataVisualization.Charting.Legend();
            System.Windows.Forms.DataVisualization.Charting.Series series1 = new System.Windows.Forms.DataVisualization.Charting.Series();
            this.splitContainerTopVsLog = new System.Windows.Forms.SplitContainer();
            this.splitContainerControlVsGraph = new System.Windows.Forms.SplitContainer();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.flowLayoutPanel1 = new System.Windows.Forms.FlowLayoutPanel();
            this.buttonInit = new System.Windows.Forms.Button();
            this.checkBoxVerbose = new System.Windows.Forms.CheckBox();
            this.buttonLaser = new System.Windows.Forms.Button();
            this.buttonStart = new System.Windows.Forms.Button();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.numericUpDownIntegrationTimeMS = new System.Windows.Forms.NumericUpDown();
            this.label1 = new System.Windows.Forms.Label();
            this.chart1 = new System.Windows.Forms.DataVisualization.Charting.Chart();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.textBoxEventLog = new System.Windows.Forms.TextBox();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainerTopVsLog)).BeginInit();
            this.splitContainerTopVsLog.Panel1.SuspendLayout();
            this.splitContainerTopVsLog.Panel2.SuspendLayout();
            this.splitContainerTopVsLog.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainerControlVsGraph)).BeginInit();
            this.splitContainerControlVsGraph.Panel1.SuspendLayout();
            this.splitContainerControlVsGraph.Panel2.SuspendLayout();
            this.splitContainerControlVsGraph.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.flowLayoutPanel1.SuspendLayout();
            this.tableLayoutPanel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownIntegrationTimeMS)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.chart1)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // splitContainerTopVsLog
            // 
            this.splitContainerTopVsLog.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainerTopVsLog.Location = new System.Drawing.Point(0, 0);
            this.splitContainerTopVsLog.Name = "splitContainerTopVsLog";
            this.splitContainerTopVsLog.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainerTopVsLog.Panel1
            // 
            this.splitContainerTopVsLog.Panel1.Controls.Add(this.splitContainerControlVsGraph);
            // 
            // splitContainerTopVsLog.Panel2
            // 
            this.splitContainerTopVsLog.Panel2.Controls.Add(this.groupBox1);
            this.splitContainerTopVsLog.Size = new System.Drawing.Size(800, 450);
            this.splitContainerTopVsLog.SplitterDistance = 266;
            this.splitContainerTopVsLog.TabIndex = 0;
            // 
            // splitContainerControlVsGraph
            // 
            this.splitContainerControlVsGraph.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainerControlVsGraph.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
            this.splitContainerControlVsGraph.Location = new System.Drawing.Point(0, 0);
            this.splitContainerControlVsGraph.Name = "splitContainerControlVsGraph";
            // 
            // splitContainerControlVsGraph.Panel1
            // 
            this.splitContainerControlVsGraph.Panel1.Controls.Add(this.groupBox2);
            // 
            // splitContainerControlVsGraph.Panel2
            // 
            this.splitContainerControlVsGraph.Panel2.Controls.Add(this.chart1);
            this.splitContainerControlVsGraph.Size = new System.Drawing.Size(800, 266);
            this.splitContainerControlVsGraph.SplitterDistance = 191;
            this.splitContainerControlVsGraph.TabIndex = 0;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.flowLayoutPanel1);
            this.groupBox2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.groupBox2.Location = new System.Drawing.Point(0, 0);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(191, 266);
            this.groupBox2.TabIndex = 0;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Controls";
            // 
            // flowLayoutPanel1
            // 
            this.flowLayoutPanel1.AutoScroll = true;
            this.flowLayoutPanel1.Controls.Add(this.buttonInit);
            this.flowLayoutPanel1.Controls.Add(this.checkBoxVerbose);
            this.flowLayoutPanel1.Controls.Add(this.buttonLaser);
            this.flowLayoutPanel1.Controls.Add(this.buttonStart);
            this.flowLayoutPanel1.Controls.Add(this.tableLayoutPanel1);
            this.flowLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.flowLayoutPanel1.Location = new System.Drawing.Point(3, 16);
            this.flowLayoutPanel1.Name = "flowLayoutPanel1";
            this.flowLayoutPanel1.Size = new System.Drawing.Size(185, 247);
            this.flowLayoutPanel1.TabIndex = 0;
            // 
            // buttonInit
            // 
            this.buttonInit.Location = new System.Drawing.Point(3, 3);
            this.buttonInit.Name = "buttonInit";
            this.buttonInit.Size = new System.Drawing.Size(75, 23);
            this.buttonInit.TabIndex = 0;
            this.buttonInit.Text = "Initialize";
            this.buttonInit.UseVisualStyleBackColor = true;
            this.buttonInit.Click += new System.EventHandler(this.buttonInit_Click);
            // 
            // checkBoxVerbose
            // 
            this.checkBoxVerbose.AutoSize = true;
            this.checkBoxVerbose.Location = new System.Drawing.Point(84, 3);
            this.checkBoxVerbose.Name = "checkBoxVerbose";
            this.checkBoxVerbose.Size = new System.Drawing.Size(65, 17);
            this.checkBoxVerbose.TabIndex = 4;
            this.checkBoxVerbose.Text = "Verbose";
            this.checkBoxVerbose.UseVisualStyleBackColor = true;
            this.checkBoxVerbose.CheckedChanged += new System.EventHandler(this.checkBoxVerbose_CheckedChanged);
            // 
            // buttonLaser
            // 
            this.buttonLaser.Location = new System.Drawing.Point(3, 32);
            this.buttonLaser.Name = "buttonLaser";
            this.buttonLaser.Size = new System.Drawing.Size(75, 23);
            this.buttonLaser.TabIndex = 1;
            this.buttonLaser.Text = "Laser On";
            this.buttonLaser.UseVisualStyleBackColor = true;
            this.buttonLaser.Click += new System.EventHandler(this.buttonLaser_Click);
            // 
            // buttonStart
            // 
            this.buttonStart.Location = new System.Drawing.Point(84, 32);
            this.buttonStart.Name = "buttonStart";
            this.buttonStart.Size = new System.Drawing.Size(75, 23);
            this.buttonStart.TabIndex = 3;
            this.buttonStart.Text = "Start";
            this.buttonStart.UseVisualStyleBackColor = true;
            this.buttonStart.Click += new System.EventHandler(this.buttonStart_Click);
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 2;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel1.Controls.Add(this.numericUpDownIntegrationTimeMS, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.label1, 1, 0);
            this.tableLayoutPanel1.Location = new System.Drawing.Point(3, 61);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 2;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(179, 100);
            this.tableLayoutPanel1.TabIndex = 2;
            // 
            // numericUpDownIntegrationTimeMS
            // 
            this.numericUpDownIntegrationTimeMS.Location = new System.Drawing.Point(3, 3);
            this.numericUpDownIntegrationTimeMS.Maximum = new decimal(new int[] {
            30000,
            0,
            0,
            0});
            this.numericUpDownIntegrationTimeMS.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numericUpDownIntegrationTimeMS.Name = "numericUpDownIntegrationTimeMS";
            this.numericUpDownIntegrationTimeMS.Size = new System.Drawing.Size(50, 20);
            this.numericUpDownIntegrationTimeMS.TabIndex = 0;
            this.numericUpDownIntegrationTimeMS.Value = new decimal(new int[] {
            10,
            0,
            0,
            0});
            this.numericUpDownIntegrationTimeMS.ValueChanged += new System.EventHandler(this.numericUpDownIntegrationTimeMS_ValueChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(59, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(79, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Integ Time (ms)";
            // 
            // chart1
            // 
            chartArea1.Name = "ChartArea1";
            this.chart1.ChartAreas.Add(chartArea1);
            this.chart1.Dock = System.Windows.Forms.DockStyle.Fill;
            legend1.Docking = System.Windows.Forms.DataVisualization.Charting.Docking.Bottom;
            legend1.Name = "Legend1";
            this.chart1.Legends.Add(legend1);
            this.chart1.Location = new System.Drawing.Point(0, 0);
            this.chart1.Name = "chart1";
            series1.ChartArea = "ChartArea1";
            series1.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Line;
            series1.Legend = "Legend1";
            series1.Name = "Series1";
            this.chart1.Series.Add(series1);
            this.chart1.Size = new System.Drawing.Size(605, 266);
            this.chart1.TabIndex = 0;
            this.chart1.Text = "chart1";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.textBoxEventLog);
            this.groupBox1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.groupBox1.Location = new System.Drawing.Point(0, 0);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(800, 180);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Event Log";
            // 
            // textBoxEventLog
            // 
            this.textBoxEventLog.Dock = System.Windows.Forms.DockStyle.Fill;
            this.textBoxEventLog.Location = new System.Drawing.Point(3, 16);
            this.textBoxEventLog.Multiline = true;
            this.textBoxEventLog.Name = "textBoxEventLog";
            this.textBoxEventLog.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.textBoxEventLog.Size = new System.Drawing.Size(794, 161);
            this.textBoxEventLog.TabIndex = 0;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 450);
            this.Controls.Add(this.splitContainerTopVsLog);
            this.Name = "Form1";
            this.Text = "Form1";
            this.splitContainerTopVsLog.Panel1.ResumeLayout(false);
            this.splitContainerTopVsLog.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainerTopVsLog)).EndInit();
            this.splitContainerTopVsLog.ResumeLayout(false);
            this.splitContainerControlVsGraph.Panel1.ResumeLayout(false);
            this.splitContainerControlVsGraph.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainerControlVsGraph)).EndInit();
            this.splitContainerControlVsGraph.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.flowLayoutPanel1.ResumeLayout(false);
            this.flowLayoutPanel1.PerformLayout();
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownIntegrationTimeMS)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.chart1)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitContainerTopVsLog;
        private System.Windows.Forms.SplitContainer splitContainerControlVsGraph;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel1;
        private System.Windows.Forms.Button buttonInit;
        private System.Windows.Forms.DataVisualization.Charting.Chart chart1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.TextBox textBoxEventLog;
        private System.Windows.Forms.Button buttonLaser;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.NumericUpDown numericUpDownIntegrationTimeMS;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button buttonStart;
        private System.Windows.Forms.CheckBox checkBoxVerbose;
    }
}

