using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Microsoft.Maker.Serial;
using Microsoft.Maker.RemoteWiring;
using System.Threading.Tasks;


// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace Door_Control
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        IStream connection;
        RemoteDevice arduino;

        public MainPage()
        {
       
            this.InitializeComponent();
            connection = new NetworkSerial(new Windows.Networking.HostName("192.168.1.170"), 3030);
            arduino = new RemoteDevice(connection);
            connection.ConnectionEstablished += OnConnectionEstablished;

            connection.begin(115200, SerialConfig.SERIAL_8N1);

        }
        private void OnConnectionEstablished()
        {
            var action = Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, new Windows.UI.Core.DispatchedHandler(() =>
            {
                LockButton.IsEnabled = true;
                UnlockButton.IsEnabled = true;

            }));
        }

        private async void LockButton_Click(object sender, RoutedEventArgs e)
        {
            arduino.digitalWrite(6, PinState.HIGH);
            await Task.Delay(10000);
            arduino.digitalWrite(6, PinState.LOW);

        }

        private async void UnlockButton_Click(object sender, RoutedEventArgs e)
        {
            arduino.digitalWrite(4, PinState.HIGH);
            await Task.Delay(10000);
            arduino.digitalWrite(4, PinState.LOW);
        }

    }
}
