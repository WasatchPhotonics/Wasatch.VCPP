using System.Text;
using System.Collections.Generic;
using System.Runtime.InteropServices;

// wonder if we should change most ints to shorts given C types 
class WasatchVCPP
{
    class Driver
    {
        public int numberOfSpectrometers;
        Dictionary<int, Spectrometer> spectrometers = new Dictionary<int, Spectrometer>();

        int openAllSpectrometers()
        {
            numberOfSpectrometers = wp_open_all_spectrometers();
            for (int i = 0; i < numberOfSpectrometers; i++)
                spectrometers.Add(i, new Spectrometer(i));
            return numberOfSpectrometers;
        }
    };

    class Spectrometer
    {
        public int specIndex;
        public string serialNumber;
        public string model;
        public int pixels;
        public double[] wavelengths;
        public double[] wavenumbers;

        public Spectrometer(int index)
        {
            specIndex = index;
            pixels = wp_get_pixels(specIndex);
            serialNumber = getSerialNumber();
            model = getSerialNumber();
        }

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

        double[] getSpectrum()
        {
            double[] result = new double[pixels];
            if (WP_SUCCESS != wp_get_spectrum(specIndex, ref result[0], pixels))
                return null;
            return result;
        }

        string byteToString(byte[] buf)
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
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_eeprom_page(int specIndex, int page, ref byte buf, int len);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_firmware_version(int specIndex, ref byte value, int len);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_fpga_version(int specIndex, ref byte value, int len);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_high_gain_mode_enable(int specIndex);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_integration_time_ms(int specIndex);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_laser_enable(int specIndex);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)] public static extern int   wp_get_library_version(ref byte , int len);
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
}

