using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Reflection;
using System.Threading;
using System.Windows.Forms;
using System.Windows.Forms.DataVisualization.Charting;

namespace WasatchVCPPNet
{
    public partial class Form1 : Form
    {
        WasatchVCPP.Driver driver;
        Dictionary<int, BackgroundWorker> workers = new Dictionary<int, BackgroundWorker>();
        Dictionary<int, Series> serieses = new Dictionary<int, Series>();
        bool laserEnabled;
        bool running;
        bool shutdownPending;

        Logger logger = Logger.getInstance();

        ////////////////////////////////////////////////////////////////////////
        // Lifecycle
        ////////////////////////////////////////////////////////////////////////

        public Form1()
        {
            InitializeComponent();

            logger.setTextBox(textBoxEventLog);

            Text = string.Format("WasatchVCPPNet {0}", Assembly.GetExecutingAssembly().GetName().Version.ToString());

        }
        
        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            shutdownPending = true;

            if (running)
                stop();

            var allComplete = true;
            foreach (var pair in workers)
            {
                if (pair.Value.IsBusy)
                {
                    allComplete = false;
                    break;
                }
            }

            if (allComplete)
                driver.closeAllSpectrometers();
            else
                e.Cancel = true;
        }  

        ////////////////////////////////////////////////////////////////////////
        // Callbacks
        ////////////////////////////////////////////////////////////////////////

        private void checkBoxVerbose_CheckedChanged(object sender, EventArgs e)
        {
            logger.level = checkBoxVerbose.Checked ? LogLevel.DEBUG : LogLevel.INFO;
        }

        private void buttonInit_Click(object sender, EventArgs e)
        {
            if (driver != null)
                return;

            chart1.Series.Clear();

            driver = new WasatchVCPP.Driver();
            driver.logfilePath = "WasatchVCPPNet.log";

            logger.info("WasatchVCPP.Driver.libraryVersion = {0}", driver.libraryVersion);

            int count = driver.openAllSpectrometers();
            logger.info($"Found {count} spectrometers");
            for (int i = 0; i < count; i++)
            {
                var spec = driver.spectrometers[i];
                logger.info("index {0} is {1} {2} with {3} pixels ({4:f2}, {5:f2}nm)",
                    i, spec.model, spec.serialNumber, spec.pixels, spec.wavelengths[0], spec.wavelengths[spec.pixels - 1]);

                Series s = new Series(spec.serialNumber);
                s.ChartType = SeriesChartType.Line;
                chart1.Series.Add(s);
                serieses.Add(i, s);

                BackgroundWorker worker = new BackgroundWorker { WorkerSupportsCancellation = true };
                worker.DoWork += Worker_DoWork;
                worker.RunWorkerCompleted += Worker_RunWorkerCompleted;
                workers.Add(i, worker);

                logger.debug("EEPROM:");
                foreach (var pair in spec.eepromFields)
                    logger.debug($"  {pair.Key} = {pair.Value}");
            }
        }

        private void buttonLaser_Click(object sender, EventArgs e)
        {
            laserEnabled = !laserEnabled;
            for (int i = 0; i < driver.numberOfSpectrometers; i++)
                driver.spectrometers[i].laserEnable = laserEnabled;
            buttonLaser.Text = laserEnabled ? "Laser Off" : "Laser On";
            logger.debug($"laserEnable -> {laserEnabled}");
        }

        private void numericUpDownIntegrationTimeMS_ValueChanged(object sender, EventArgs e)
        {
            var ms = (uint)numericUpDownIntegrationTimeMS.Value;
            for (int i = 0; i < driver.numberOfSpectrometers; i++)
                driver.spectrometers[i].integrationTimeMS = ms;
            logger.debug($"integrationTimeMS -> {ms}");
        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            if (running)
                stop();
            else
                start();
        }

        ////////////////////////////////////////////////////////////////////////
        // Methods
        ////////////////////////////////////////////////////////////////////////

        void start()
        {
            running = true;
            buttonStart.Text = "Stop";
            for (int i = 0; i < driver.numberOfSpectrometers; i++)
                workers[i].RunWorkerAsync(i);
        }

        void stop()
        {
            for (int i = 0; i < driver.numberOfSpectrometers; i++)
                workers[i].CancelAsync();
        }

        void processSpectrum(int index, double[] spectrum)
        {
            var spec = driver.spectrometers[index];
            var series = serieses[index];
            if (spec is null || series is null)
                return;

            var x = spec.wavelengths;
            var y = spectrum;
            if (x is null || y is null)
                return;

            series.Points.DataBindXY(x, y);
        }

        ////////////////////////////////////////////////////////////////////////
        // Background Workers
        ////////////////////////////////////////////////////////////////////////

        private void Worker_DoWork(object sender, DoWorkEventArgs e)
        {
            int index = (int)e.Argument;
            var spec = driver.spectrometers[index];
            var worker = workers[index];

            while(true)
            {
                if (worker.CancellationPending)
                {
                    e.Cancel = true;
                    break;
                }

                var spectrum = spec.getSpectrum();
                chart1.BeginInvoke(new MethodInvoker(delegate { processSpectrum(index, spectrum); }));

                Thread.Sleep(100);
            }
        }

        private void Worker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            var allComplete = true;
            foreach (var pair in workers)
                if (pair.Value.IsBusy)
                {
                    allComplete = false;
                    break;
                }

            if (allComplete)
            {
                running = false;
                buttonStart.Text = "Start";

                if (shutdownPending)
                    Close();
            }
        }
    }
}
