﻿namespace WasatchVCPPNet
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
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea2 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Legend legend2 = new System.Windows.Forms.DataVisualization.Charting.Legend();
            System.Windows.Forms.DataVisualization.Charting.Series series2 = new System.Windows.Forms.DataVisualization.Charting.Series();
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
            this.buttonMetadata = new System.Windows.Forms.Button();
            this.numericUpDownMaxTimeoutMS = new System.Windows.Forms.NumericUpDown();
            this.label2 = new System.Windows.Forms.Label();
            this.checkBoxTECEnable = new System.Windows.Forms.CheckBox();
            this.checkBoxHighGainModeEnable = new System.Windows.Forms.CheckBox();
            this.numericUpDownTECSetpointDegC = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.numericUpDownDetectorOffset = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownDetectorGainOdd = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownDetectorGain = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownDetectorOffsetOdd = new System.Windows.Forms.NumericUpDown();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
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
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownMaxTimeoutMS)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownTECSetpointDegC)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownDetectorOffset)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownDetectorGainOdd)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownDetectorGain)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownDetectorOffsetOdd)).BeginInit();
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
            this.buttonLaser.Location = new System.Drawing.Point(3, 61);
            this.buttonLaser.Name = "buttonLaser";
            this.buttonLaser.Size = new System.Drawing.Size(75, 23);
            this.buttonLaser.TabIndex = 1;
            this.buttonLaser.Text = "Laser On";
            this.buttonLaser.UseVisualStyleBackColor = true;
            this.buttonLaser.Click += new System.EventHandler(this.buttonLaser_Click);
            // 
            // buttonStart
            // 
            this.buttonStart.Location = new System.Drawing.Point(3, 32);
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
            this.tableLayoutPanel1.Controls.Add(this.buttonStart, 0, 1);
            this.tableLayoutPanel1.Controls.Add(this.buttonLaser, 0, 2);
            this.tableLayoutPanel1.Controls.Add(this.checkBoxVerbose, 1, 0);
            this.tableLayoutPanel1.Controls.Add(this.buttonInit, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.buttonMetadata, 1, 1);
            this.tableLayoutPanel1.Controls.Add(this.checkBoxHighGainModeEnable, 1, 3);
            this.tableLayoutPanel1.Controls.Add(this.checkBoxTECEnable, 0, 3);
            this.tableLayoutPanel1.Controls.Add(this.numericUpDownIntegrationTimeMS, 0, 4);
            this.tableLayoutPanel1.Controls.Add(this.label1, 1, 4);
            this.tableLayoutPanel1.Controls.Add(this.numericUpDownMaxTimeoutMS, 0, 5);
            this.tableLayoutPanel1.Controls.Add(this.label2, 1, 5);
            this.tableLayoutPanel1.Controls.Add(this.numericUpDownTECSetpointDegC, 0, 6);
            this.tableLayoutPanel1.Controls.Add(this.label3, 1, 6);
            this.tableLayoutPanel1.Controls.Add(this.numericUpDownDetectorOffset, 0, 9);
            this.tableLayoutPanel1.Controls.Add(this.numericUpDownDetectorGainOdd, 0, 8);
            this.tableLayoutPanel1.Controls.Add(this.numericUpDownDetectorGain, 0, 7);
            this.tableLayoutPanel1.Controls.Add(this.numericUpDownDetectorOffsetOdd, 0, 10);
            this.tableLayoutPanel1.Controls.Add(this.label4, 1, 7);
            this.tableLayoutPanel1.Controls.Add(this.label5, 1, 8);
            this.tableLayoutPanel1.Controls.Add(this.label6, 1, 9);
            this.tableLayoutPanel1.Controls.Add(this.label7, 1, 10);
            this.tableLayoutPanel1.Location = new System.Drawing.Point(3, 3);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 11;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.Size = new System.Drawing.Size(179, 320);
            this.tableLayoutPanel1.TabIndex = 2;
            // 
            // numericUpDownIntegrationTimeMS
            // 
            this.numericUpDownIntegrationTimeMS.Location = new System.Drawing.Point(3, 113);
            this.numericUpDownIntegrationTimeMS.Maximum = new decimal(new int[] {
            600000,
            0,
            0,
            0});
            this.numericUpDownIntegrationTimeMS.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numericUpDownIntegrationTimeMS.Name = "numericUpDownIntegrationTimeMS";
            this.numericUpDownIntegrationTimeMS.Size = new System.Drawing.Size(70, 20);
            this.numericUpDownIntegrationTimeMS.TabIndex = 0;
            this.numericUpDownIntegrationTimeMS.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
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
            this.label1.Location = new System.Drawing.Point(84, 110);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(79, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Integ Time (ms)";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // chart1
            // 
            chartArea2.CursorX.IsUserEnabled = true;
            chartArea2.CursorX.IsUserSelectionEnabled = true;
            chartArea2.CursorY.IsUserEnabled = true;
            chartArea2.CursorY.IsUserSelectionEnabled = true;
            chartArea2.Name = "ChartArea1";
            this.chart1.ChartAreas.Add(chartArea2);
            this.chart1.Dock = System.Windows.Forms.DockStyle.Fill;
            legend2.Alignment = System.Drawing.StringAlignment.Center;
            legend2.Docking = System.Windows.Forms.DataVisualization.Charting.Docking.Bottom;
            legend2.Name = "Legend1";
            this.chart1.Legends.Add(legend2);
            this.chart1.Location = new System.Drawing.Point(0, 0);
            this.chart1.Name = "chart1";
            series2.ChartArea = "ChartArea1";
            series2.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Line;
            series2.Legend = "Legend1";
            series2.Name = "Series1";
            this.chart1.Series.Add(series2);
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
            this.textBoxEventLog.Font = new System.Drawing.Font("Consolas", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.textBoxEventLog.Location = new System.Drawing.Point(3, 16);
            this.textBoxEventLog.Multiline = true;
            this.textBoxEventLog.Name = "textBoxEventLog";
            this.textBoxEventLog.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.textBoxEventLog.Size = new System.Drawing.Size(794, 161);
            this.textBoxEventLog.TabIndex = 0;
            // 
            // buttonMetadata
            // 
            this.buttonMetadata.Location = new System.Drawing.Point(84, 32);
            this.buttonMetadata.Name = "buttonMetadata";
            this.buttonMetadata.Size = new System.Drawing.Size(75, 23);
            this.buttonMetadata.TabIndex = 5;
            this.buttonMetadata.Text = "Metadata";
            this.buttonMetadata.UseVisualStyleBackColor = true;
            this.buttonMetadata.Click += new System.EventHandler(this.buttonMetadata_Click);
            // 
            // numericUpDownMaxTimeoutMS
            // 
            this.numericUpDownMaxTimeoutMS.Location = new System.Drawing.Point(3, 139);
            this.numericUpDownMaxTimeoutMS.Maximum = new decimal(new int[] {
            30000,
            0,
            0,
            0});
            this.numericUpDownMaxTimeoutMS.Name = "numericUpDownMaxTimeoutMS";
            this.numericUpDownMaxTimeoutMS.Size = new System.Drawing.Size(70, 20);
            this.numericUpDownMaxTimeoutMS.TabIndex = 6;
            this.numericUpDownMaxTimeoutMS.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.numericUpDownMaxTimeoutMS.Value = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.numericUpDownMaxTimeoutMS.ValueChanged += new System.EventHandler(this.numericUpDownMaxTimeoutMS_ValueChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(84, 136);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(90, 13);
            this.label2.TabIndex = 7;
            this.label2.Text = "Max Timeout (ms)";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // checkBoxTECEnable
            // 
            this.checkBoxTECEnable.AutoSize = true;
            this.checkBoxTECEnable.Location = new System.Drawing.Point(3, 90);
            this.checkBoxTECEnable.Name = "checkBoxTECEnable";
            this.checkBoxTECEnable.Size = new System.Drawing.Size(47, 17);
            this.checkBoxTECEnable.TabIndex = 8;
            this.checkBoxTECEnable.Text = "TEC";
            this.checkBoxTECEnable.UseVisualStyleBackColor = true;
            this.checkBoxTECEnable.CheckedChanged += new System.EventHandler(this.checkBoxTECEnable_CheckedChanged);
            // 
            // checkBoxHighGainModeEnable
            // 
            this.checkBoxHighGainModeEnable.AutoSize = true;
            this.checkBoxHighGainModeEnable.Location = new System.Drawing.Point(84, 90);
            this.checkBoxHighGainModeEnable.Name = "checkBoxHighGainModeEnable";
            this.checkBoxHighGainModeEnable.Size = new System.Drawing.Size(73, 17);
            this.checkBoxHighGainModeEnable.TabIndex = 9;
            this.checkBoxHighGainModeEnable.Text = "High Gain";
            this.checkBoxHighGainModeEnable.UseVisualStyleBackColor = true;
            this.checkBoxHighGainModeEnable.CheckedChanged += new System.EventHandler(this.checkBoxHighGainModeEnable_CheckedChanged);
            // 
            // numericUpDownTECSetpointDegC
            // 
            this.numericUpDownTECSetpointDegC.Location = new System.Drawing.Point(3, 165);
            this.numericUpDownTECSetpointDegC.Maximum = new decimal(new int[] {
            60,
            0,
            0,
            0});
            this.numericUpDownTECSetpointDegC.Minimum = new decimal(new int[] {
            25,
            0,
            0,
            -2147483648});
            this.numericUpDownTECSetpointDegC.Name = "numericUpDownTECSetpointDegC";
            this.numericUpDownTECSetpointDegC.Size = new System.Drawing.Size(70, 20);
            this.numericUpDownTECSetpointDegC.TabIndex = 10;
            this.numericUpDownTECSetpointDegC.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.numericUpDownTECSetpointDegC.ValueChanged += new System.EventHandler(this.numericUpDownTECSetpointDegC_ValueChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(84, 162);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(84, 13);
            this.label3.TabIndex = 11;
            this.label3.Text = "TEC Setpoint °C";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // numericUpDownDetectorOffset
            // 
            this.numericUpDownDetectorOffset.Location = new System.Drawing.Point(3, 243);
            this.numericUpDownDetectorOffset.Maximum = new decimal(new int[] {
            20000,
            0,
            0,
            0});
            this.numericUpDownDetectorOffset.Minimum = new decimal(new int[] {
            20000,
            0,
            0,
            -2147483648});
            this.numericUpDownDetectorOffset.Name = "numericUpDownDetectorOffset";
            this.numericUpDownDetectorOffset.Size = new System.Drawing.Size(70, 20);
            this.numericUpDownDetectorOffset.TabIndex = 12;
            this.numericUpDownDetectorOffset.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.numericUpDownDetectorOffset.ValueChanged += new System.EventHandler(this.numericUpDownDetectorOffset_ValueChanged);
            // 
            // numericUpDownDetectorGainOdd
            // 
            this.numericUpDownDetectorGainOdd.DecimalPlaces = 2;
            this.numericUpDownDetectorGainOdd.Location = new System.Drawing.Point(3, 217);
            this.numericUpDownDetectorGainOdd.Name = "numericUpDownDetectorGainOdd";
            this.numericUpDownDetectorGainOdd.Size = new System.Drawing.Size(70, 20);
            this.numericUpDownDetectorGainOdd.TabIndex = 13;
            this.numericUpDownDetectorGainOdd.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.numericUpDownDetectorGainOdd.ValueChanged += new System.EventHandler(this.numericUpDownDetectorGainOdd_ValueChanged);
            // 
            // numericUpDownDetectorGain
            // 
            this.numericUpDownDetectorGain.DecimalPlaces = 2;
            this.numericUpDownDetectorGain.Location = new System.Drawing.Point(3, 191);
            this.numericUpDownDetectorGain.Name = "numericUpDownDetectorGain";
            this.numericUpDownDetectorGain.Size = new System.Drawing.Size(70, 20);
            this.numericUpDownDetectorGain.TabIndex = 14;
            this.numericUpDownDetectorGain.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.numericUpDownDetectorGain.ValueChanged += new System.EventHandler(this.numericUpDownDetectorGain_ValueChanged);
            // 
            // numericUpDownDetectorOffsetOdd
            // 
            this.numericUpDownDetectorOffsetOdd.Location = new System.Drawing.Point(3, 269);
            this.numericUpDownDetectorOffsetOdd.Maximum = new decimal(new int[] {
            20000,
            0,
            0,
            0});
            this.numericUpDownDetectorOffsetOdd.Minimum = new decimal(new int[] {
            20000,
            0,
            0,
            -2147483648});
            this.numericUpDownDetectorOffsetOdd.Name = "numericUpDownDetectorOffsetOdd";
            this.numericUpDownDetectorOffsetOdd.Size = new System.Drawing.Size(70, 20);
            this.numericUpDownDetectorOffsetOdd.TabIndex = 15;
            this.numericUpDownDetectorOffsetOdd.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.numericUpDownDetectorOffsetOdd.ValueChanged += new System.EventHandler(this.numericUpDownDetectorOffsetOdd_ValueChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(84, 188);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(29, 13);
            this.label4.TabIndex = 16;
            this.label4.Text = "Gain";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(84, 214);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(56, 13);
            this.label5.TabIndex = 17;
            this.label5.Text = "Gain (odd)";
            this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(84, 240);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(35, 13);
            this.label6.TabIndex = 18;
            this.label6.Text = "Offset";
            this.label6.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(84, 266);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(62, 13);
            this.label7.TabIndex = 19;
            this.label7.Text = "Offset (odd)";
            this.label7.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
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
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownIntegrationTimeMS)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.chart1)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownMaxTimeoutMS)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownTECSetpointDegC)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownDetectorOffset)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownDetectorGainOdd)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownDetectorGain)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownDetectorOffsetOdd)).EndInit();
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
        private System.Windows.Forms.Button buttonMetadata;
        private System.Windows.Forms.NumericUpDown numericUpDownMaxTimeoutMS;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.CheckBox checkBoxHighGainModeEnable;
        private System.Windows.Forms.CheckBox checkBoxTECEnable;
        private System.Windows.Forms.NumericUpDown numericUpDownTECSetpointDegC;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.NumericUpDown numericUpDownDetectorOffset;
        private System.Windows.Forms.NumericUpDown numericUpDownDetectorGainOdd;
        private System.Windows.Forms.NumericUpDown numericUpDownDetectorGain;
        private System.Windows.Forms.NumericUpDown numericUpDownDetectorOffsetOdd;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
    }
}

