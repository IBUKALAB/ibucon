using System;
using System.IO.Ports;
using System.Text;
using System.Windows;
using System.Windows.Controls;

namespace USBHIDConfigurator
{
    public partial class MainWindow : Window
    {
        private SerialPort serialPort;

        public MainWindow()
        {
            InitializeComponent();
            serialPort = new SerialPort("COM7", 115200);
            serialPort.Open();
        }

        private void SendConfig(object sender, RoutedEventArgs e)
        {
            StringBuilder configString = new StringBuilder();

            configString.Append("B1:");
            configString.Append(FormatKeyConfig(Switch1Mod1, Switch1Mod2, Switch1Key));
            configString.Append(";");

            configString.Append("B2:");
            configString.Append(FormatKeyConfig(Switch2Mod1, Switch2Mod2, Switch2Key));
            configString.Append(";");

            configString.Append("B3:");
            configString.Append(FormatKeyConfig(Switch3Mod1, Switch3Mod2, Switch3Key));
            configString.Append(";");

            configString.Append("W1:");
            configString.Append(FormatWheelConfig(Wheel1Mod, Wheel1Scroll));
            configString.Append(";");

            configString.Append("W2:");
            configString.Append(FormatWheelConfig(Wheel2Mod, Wheel2Scroll));
            configString.Append(";");

            try
            {
                serialPort.WriteLine(configString.ToString());
                MessageBox.Show("設定を送信しました");
            }
            catch (Exception ex)
            {
                MessageBox.Show($"シリアルポートへの書き込み中にエラーが発生しました: {ex.Message}");
            }
        }

        private string FormatKeyConfig(ComboBox mod1, ComboBox mod2, ComboBox key)
        {
            return $"{((ComboBoxItem)mod1.SelectedItem).Content}+{((ComboBoxItem)mod2.SelectedItem).Content}+{((ComboBoxItem)key.SelectedItem).Content}";
        }

        private string FormatWheelConfig(ComboBox mod, ComboBox scroll)
        {
            return $"{((ComboBoxItem)mod.SelectedItem).Content}+{((ComboBoxItem)scroll.SelectedItem).Content}";
        }
    }
}
