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
        BackgroundWorker workerTemperature;
        Dictionary<int, BackgroundWorker> workers = new Dictionary<int, BackgroundWorker>();
        Dictionary<int, Series> serieses = new Dictionary<int, Series>();
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

            checkBoxMonitorTemperature.Checked = false;
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
            {
                if (driver != null)
                    driver.closeAllSpectrometers();
            }
            else
                e.Cancel = true;
        }  

        ////////////////////////////////////////////////////////////////////////
        // Callbacks
        ////////////////////////////////////////////////////////////////////////

        void checkBoxVerbose_CheckedChanged(object sender, EventArgs e)
        {
            var verbose = checkBoxVerbose.Checked;
            logger.level = verbose ? LogLevel.DEBUG : LogLevel.INFO;
            if (driver != null)
                driver.logLevel = verbose ? 0 : 1;
        }

        void buttonInit_Click(object sender, EventArgs e)
        {
            if (driver != null)
                return;

            chart1.Series.Clear();

            driver = new WasatchVCPP.Driver();

            driver.logLevel = checkBoxVerbose.Checked ? 0 : 1;
            driver.logfilePath = "WasatchVCPPNet.log";

            logger.info("WasatchVCPP.Driver.libraryVersion = {0}", driver.libraryVersion);

            int count = driver.openAllSpectrometers();
            logger.info($"Found {count} spectrometers");
            for (int i = 0; i < count; i++)
            {
                var spec = driver.spectrometers[i];

                string wavenumbers = "";
                if (spec.wavenumbers != null)
                    wavenumbers = string.Format(" ({0:f2}, {1:f2}cm⁻¹)", spec.wavenumbers[0], spec.wavenumbers[spec.pixels - 1]);

                logger.info("index {0} is {1} {2} with {3} pixels ({4:f2}, {5:f2}nm){6} firmware {7} FPGA {8}",
                    i, spec.model, spec.serialNumber, spec.pixels, 
                    spec.wavelengths[0], spec.wavelengths[spec.pixels - 1],
                    wavenumbers,
                    spec.firmwareVersion, spec.fpgaVersion);

                Series s = new Series(spec.serialNumber);
                s.ChartType = SeriesChartType.Line;
                chart1.Series.Add(s);
                serieses.Add(i, s);

                BackgroundWorker worker = new BackgroundWorker { WorkerSupportsCancellation = true };
                worker.DoWork += Worker_DoWork;
                worker.RunWorkerCompleted += Worker_RunWorkerCompleted;
                workers.Add(i, worker);
            }

            initGUIFromFirstSpectrometer();

            workerTemperature = new BackgroundWorker { WorkerSupportsCancellation = true };
            workerTemperature.DoWork += WorkerTemperature_DoWork;
            workerTemperature.RunWorkerCompleted += WorkerTemperature_RunWorkerCompleted;

            buttonStart.Enabled = buttonMetadata.Enabled = true;
        }

        void buttonMetadata_Click(object sender, EventArgs e)
        {
            if (!initialized)
                return;

            for (int i = 0; i < driver.numberOfSpectrometers; i++)
            {
                var spec = driver.spectrometers[i];

                logger.info($"Metadata for {spec.serialNumber}");
                logger.info($"  Integration Time:       {spec.integrationTimeMS} ms");
                logger.info($"  Laser Enable:           {spec.laserEnable} ms");
                logger.info($"  Temperature:            {spec.detectorTemperatureDegC:f2} °C");
                logger.info($"  Detector TEC Enable:    {spec.detectorTECEnable}");
                logger.info($"  Detector TEC Setpoint:  {spec.detectorTECSetpointDegC}");
                logger.info($"  Detector Gain:          {spec.detectorGain:f2}");
                logger.info($"  Detector Gain Odd:      {spec.detectorGainOdd:f2}");
                logger.info($"  Detector Offset:        {spec.detectorOffset}");
                logger.info($"  Detector Offset Odd:    {spec.detectorOffsetOdd}");
                logger.info($"  High-Gain Mode Enable:  {spec.highGainModeEnable}");
                logger.info($"  Max Timeout (ms):       {spec.maxTimeoutMS}");

                logger.info("");
                logger.info("  EEPROM:");
                foreach (var pair in spec.eepromFields)
                    logger.info($"    {pair.Key} = {pair.Value}");

                logger.info("");
                for (int page = 0; page < 8; page++)
                    logger.hexdump(spec.getEEPROMPage(page),
                                   string.Format("    buf[{0}]: ", page));
            }
            driver.logDebug("sample injected log debug message");
        }

        void checkBoxLaserEnable_CheckedChanged(object sender, EventArgs e)
        {
            if (!initialized)
                return;

            var enabled = (sender as CheckBox).Checked;
            for (int i = 0; i < driver.numberOfSpectrometers; i++)
                driver.spectrometers[i].laserEnable = enabled;
        }

        void numericUpDownIntegrationTimeMS_ValueChanged(object sender, EventArgs e)
        {
            if (!initialized)
                return;

            var ms = (uint)numericUpDownIntegrationTimeMS.Value;
            for (int i = 0; i < driver.numberOfSpectrometers; i++)
                driver.spectrometers[i].integrationTimeMS = ms;
        }

        void numericUpDownMaxTimeoutMS_ValueChanged(object sender, EventArgs e)
        {
            if (!initialized)
                return;

            var ms = (int)numericUpDownMaxTimeoutMS.Value;
            for (int i = 0; i < driver.numberOfSpectrometers; i++)
                driver.spectrometers[i].maxTimeoutMS = ms;
        }

        void numericUpDownTECSetpointDegC_ValueChanged(object sender, EventArgs e)
        {
            if (!initialized)
                return;

            var degC = (int)numericUpDownTECSetpointDegC.Value;
            for (int i = 0; i < driver.numberOfSpectrometers; i++)
                driver.spectrometers[i].detectorTECSetpointDegC = degC;
        }

        void checkBoxTECEnable_CheckedChanged(object sender, EventArgs e)
        {
            if (!initialized)
                return;

            var enable = checkBoxTECEnable.Checked;
            for (int i = 0; i < driver.numberOfSpectrometers; i++)
                driver.spectrometers[i].detectorTECEnable = enable;
        }

        void checkBoxHighGainModeEnable_CheckedChanged(object sender, EventArgs e)
        {
            if (!initialized)
                return;

            var enable = checkBoxHighGainModeEnable.Checked;
            for (int i = 0; i < driver.numberOfSpectrometers; i++)
                driver.spectrometers[i].highGainModeEnable = enable;
        }

        void numericUpDownDetectorGain_ValueChanged(object sender, EventArgs e)
        {
            if (!initialized)
                return;

            var value = (float)(sender as NumericUpDown).Value;
            for (int i = 0; i < driver.numberOfSpectrometers; i++)
                driver.spectrometers[i].detectorGain = value;
        }

        void numericUpDownDetectorGainOdd_ValueChanged(object sender, EventArgs e)
        {
            if (!initialized)
                return;

            var value = (float)(sender as NumericUpDown).Value;
            for (int i = 0; i < driver.numberOfSpectrometers; i++)
                driver.spectrometers[i].detectorGainOdd = value;
        }

        void numericUpDownDetectorOffset_ValueChanged(object sender, EventArgs e)
        {
            if (!initialized)
                return;

            var value = (int)(sender as NumericUpDown).Value;
            for (int i = 0; i < driver.numberOfSpectrometers; i++)
                driver.spectrometers[i].detectorOffset = value;
        }

        void numericUpDownDetectorOffsetOdd_ValueChanged(object sender, EventArgs e)
        {
            if (!initialized)
                return;

            var value = (int)(sender as NumericUpDown).Value;
            for (int i = 0; i < driver.numberOfSpectrometers; i++)
                driver.spectrometers[i].detectorOffsetOdd = value;
        }

        void buttonStart_Click(object sender, EventArgs e)
        {
            if (!initialized)
                return;

            if (running)
                stop();
            else
                start();
        }

        void checkBoxMonitorTemperature_CheckedChanged(object sender, EventArgs e)
        {
            var enabled = (sender as CheckBox).Checked;
            if (enabled)
                workerTemperature.RunWorkerAsync();
            else
                workerTemperature.CancelAsync();
        }

        ////////////////////////////////////////////////////////////////////////
        // Methods
        ////////////////////////////////////////////////////////////////////////

        bool initialized => driver != null && driver.numberOfSpectrometers > 0;

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
            {
                workers[i].CancelAsync();
                driver.spectrometers[i].cancelOperation();
            }
        }

        void processSpectrum(int index, double[] spectrum)
        {
            if (!initialized)
                return;

            var spec = driver.spectrometers[index];
            var series = serieses[index];
            if (spec is null || series is null)
                return;

            var x = spec.wavelengths;
            var y = spectrum;
            if (x is null || y is null)
                return;

            series.Points.DataBindXY(x, y);

            chart1.ChartAreas[0].AxisY.IsStartedFromZero = false;
            chart1.ChartAreas[0].RecalculateAxesScale();
        }

        void initGUIFromFirstSpectrometer()
        {
            if (!initialized)
                return;
            var spec = driver.spectrometers[0];

            // these will trigger a series of callbacks, which will have
            // the effect of setting all connected spectrometers to the same
            // settings as the first one (skipping TEC setpoint, as that
            // would rarely be optimal for heterogenous testbeds)

            checkBoxLaserEnable.Checked = spec.laserEnable;
            numericUpDownIntegrationTimeMS.Value = spec.integrationTimeMS;
            numericUpDownMaxTimeoutMS.Value = spec.maxTimeoutMS;
            checkBoxTECEnable.Checked = spec.detectorTECEnable;
            checkBoxHighGainModeEnable.Checked = spec.highGainModeEnable;

            // only update these to the GUI if there's only one connected spectrometer
            if (driver.numberOfSpectrometers == 1)
            {
                numericUpDownTECSetpointDegC.Value = spec.detectorTECSetpointDegC;
                numericUpDownDetectorGain.Value = (decimal)spec.detectorGain;
                numericUpDownDetectorGainOdd.Value = (decimal)spec.detectorGainOdd;
                numericUpDownDetectorOffset.Value = spec.detectorOffset;
                numericUpDownDetectorOffsetOdd.Value = spec.detectorOffsetOdd;
            }
            else
            {
                // just disable these widgets when connected to multiple spectrometers,
                // until we provide a drop-down menu allowing selection of one spectrometer
                numericUpDownTECSetpointDegC.Enabled = false;
                numericUpDownDetectorGain.Enabled = false;
                numericUpDownDetectorGainOdd.Enabled = false;
                numericUpDownDetectorOffset.Enabled = false;
                numericUpDownDetectorOffsetOdd.Enabled = false;
            }
        }

        ////////////////////////////////////////////////////////////////////////
        // Acquisition Worker
        ////////////////////////////////////////////////////////////////////////

        void Worker_DoWork(object sender, DoWorkEventArgs e)
        {
            if (!initialized)
                return;

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

        void Worker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            var allComplete = true;
            foreach (var pair in workers)
                if (pair.Value.IsBusy)
                    allComplete = false;

            if (allComplete)
            {
                running = false;
                buttonStart.Text = "Start";

                if (shutdownPending)
                    Close();
            }
        }

        ////////////////////////////////////////////////////////////////////////
        // Temperature Worker
        ////////////////////////////////////////////////////////////////////////

        void WorkerTemperature_DoWork(object sender, DoWorkEventArgs e)
        {
            if (!initialized)
                return;
            var spec = driver.spectrometers[0];

            while (true)
            {
                if (workerTemperature.CancellationPending || shutdownPending)
                    break;

                var degC = spec.detectorTemperatureDegC;
                chart1.BeginInvoke(new MethodInvoker(delegate { labelDetectorTemperatureDegC.Text = string.Format("{0:f2}", degC); }));

                Thread.Sleep(1000);
            }
        }

        void WorkerTemperature_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (!shutdownPending)
                labelDetectorTemperatureDegC.Text = "disabled";
        }
    }
}
