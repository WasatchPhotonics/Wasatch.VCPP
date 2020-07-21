using System.Text;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.CodeDom.Compiler;
using System;

// wonder if we should change most ints to shorts given C types 
class WasatchVCPP
{
    public class Driver
    {
        public Dictionary<int, Spectrometer> spectrometers = new Dictionary<int, Spectrometer>();
        public int numberOfSpectrometers;

        public string libraryVersion
        {
            get
            {
                byte[] buf = new byte[16];
                wp_get_library_version(ref buf[0], buf.Length);
                return Spectrometer.byteToString(buf);
            }
        }

        public string logfilePath
        {
            set
            {
                byte[] buf = Spectrometer.stringToBytes(value);  
                wp_set_logfile_path(ref buf[0]);
            }
        }

        public int openAllSpectrometers()
        {
            numberOfSpectrometers = wp_open_all_spectrometers();
            for (int i = 0; i < numberOfSpectrometers; i++)
                spectrometers.Add(i, new Spectrometer(i));
            return numberOfSpectrometers;
        }

        public void closeAllSpectrometers() => wp_close_all_spectrometers();
    };

    public class Spectrometer
    {
        public int specIndex;
        public string serialNumber;
        public string model;
        public int pixels;
        public double[] wavelengths;
        public double[] wavenumbers;
        public SortedDictionary<string, string> eepromFields = new SortedDictionary<string, string>();

        public Spectrometer(int index)
        {
            specIndex = index;
            pixels = wp_get_pixels(specIndex);
            serialNumber = getSerialNumber();
            model = getModel();
            wavelengths = getWavelengths();
            wavenumbers = getWavenumbers();

            readEEPROM();
        }

        ////////////////////////////////////////////////////////////////////////
        // private accessors
        ////////////////////////////////////////////////////////////////////////

        string getSerialNumber()
        {
            byte[] buf = new byte[17];
            if (WP_SUCCESS != wp_get_serial_number(specIndex, ref buf[0], buf.Length))
                return null;
            return byteToString(buf);
        }

        string getModel()
        {
            byte[] buf = new byte[33];
            if (WP_SUCCESS != wp_get_model(specIndex, ref buf[0], buf.Length))
                return null;
            return byteToString(buf);
        }

        double[] getWavelengths()
        {
            double[] result = new double[pixels];
            if (WP_SUCCESS != wp_get_wavelengths(specIndex, ref result[0], pixels))
                return null;
            return result;
        }

        double[] getWavenumbers()
        {
            double[] result = new double[pixels];
            if (WP_SUCCESS != wp_get_wavenumbers(specIndex, ref result[0], pixels))
                return null;
            return result;
        }

        ////////////////////////////////////////////////////////////////////////
        // public accessors
        ////////////////////////////////////////////////////////////////////////

        public bool laserEnable
        {
            get => 0 != wp_get_laser_enable(specIndex);
            set => wp_set_laser_enable(specIndex, value ? 1 : 0);
        }

        public uint integrationTimeMS
        {
            get => (uint)wp_get_integration_time_ms(specIndex);
            set => wp_set_integration_time_ms(specIndex, value);
        }

        public double[] getSpectrum()
        {
            double[] result = new double[pixels];
            if (WP_SUCCESS != wp_get_spectrum(specIndex, ref result[0], pixels))
                return null;
            return result;
        }

        ////////////////////////////////////////////////////////////////////////
        // EEPROM
        ////////////////////////////////////////////////////////////////////////

        public int eepromFieldCount => wp_get_eeprom_field_count(specIndex);

        public string getEEPROMFieldName(int index)
        {
            byte[] value = new byte[64];
            if (WP_SUCCESS == wp_get_eeprom_field_name(specIndex, index, ref value[0], value.Length))
                return byteToString(value);
            return null;
        }

        public string getEEPROMField(string name)
        {
            byte[] bufName = Spectrometer.stringToBytes(name);  
            byte[] value = new byte[64];

            if (WP_SUCCESS == wp_get_eeprom_field(specIndex, ref bufName[0], ref value[0], value.Length))
                return byteToString(value);

            return null;
        }

        void readEEPROM()
        {
            eepromFields.Clear();
            var count = eepromFieldCount;
            for (var i = 0; i < count; i++)
            {
                var name = getEEPROMFieldName(i);
                if (name != "userData")
                {
                    Console.WriteLine($"field {i} = {name}");
                    if (name != null)
                        eepromFields[name] = getEEPROMField(name);
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////
        // utility
        ////////////////////////////////////////////////////////////////////////

        //! @see https://stackoverflow.com/a/2794356/11615696
        static public byte[] stringToBytes(string s)
        {
            string line = s + char.MinValue; // Add null terminator.
            byte[] buf = ASCIIEncoding.ASCII.GetBytes(line);
            return buf;
        }

        static public string byteToString(byte[] buf)
        {   
            int len = 0;
            while (buf[len] != 0 && len + 1 < buf.Length)
                len++;  

            byte[] clean = Encoding.Convert(Encoding.GetEncoding("iso-8859-1"), Encoding.UTF8, buf);
            return Encoding.UTF8.GetString(clean, 0, len);
        }  
    };

    ////////////////////////////////////////////////////////////////////////////
    // C API
    ////////////////////////////////////////////////////////////////////////////

    const string DLL = "WasatchVCPP.dll";
    public const int WP_SUCCESS = 0;

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern float wp_get_detector_gain(int specIndex);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern float wp_get_detector_gain_odd(int specIndex);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern float wp_get_detector_tec_setpoint_deg_c(int specIndex);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern float wp_get_detector_temperature_deg_c(int specIndex);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_close_all_spectrometers();
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_close_spectrometer(int specIndex);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_detector_offset(int specIndex);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_detector_offset_odd(int specIndex);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_detector_tec_enable(int specIndex);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_eeprom(int specIndex, ref byte names, ref byte values, int len);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_eeprom_field(int specIndex, ref byte name, ref byte value, int len);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_eeprom_field_count(int specIndex);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_eeprom_field_name(int specIndex, int index, ref byte name, int len);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_eeprom_page(int specIndex, int page, ref byte buf, int len);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_firmware_version(int specIndex, ref byte value, int len);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_fpga_version(int specIndex, ref byte value, int len);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_high_gain_mode_enable(int specIndex);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_integration_time_ms(int specIndex);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_laser_enable(int specIndex);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_library_version(ref byte value, int len);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_model(int specIndex, ref byte value, int len);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_number_of_spectrometers();
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_pixels(int specIndex);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_serial_number(int specIndex, ref byte value, int len);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_spectrum(int specIndex, ref double spectrum, int len);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_wavelengths(int specIndex, ref double wavelengths, int len);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_wavenumbers(int specIndex, ref double wavenumbers, int len);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_open_all_spectrometers();
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_read_control_msg(byte bRequest, ushort wIndex, ref byte data, int len, int fullLen);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_send_control_msg(byte bRequest, ushort wValue, ushort wIndex, ref byte data, int len);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_set_detector_gain(int specIndex, float value);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_set_detector_gain_odd(int specIndex, float value);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_set_detector_offset(int specIndex, int value);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_set_detector_offset_odd(int specIndex, int value);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_set_detector_tec_enable(int specIndex, int value);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_set_detector_tec_setpoint_deg_c(int specIndex, int value);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_set_high_gain_mode_enable(int specIndex, int value);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_set_integration_time_ms(int specIndex, uint ms);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_set_laser_enable(int specIndex, int value);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_set_log_level(int level);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_set_logfile_path(ref byte pathname);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_cancel_operation(int specIndex);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_set_max_timeout_ms(int specIndex, int maxTimeoutMS);
}

