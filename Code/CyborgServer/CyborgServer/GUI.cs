using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CyborgServer
{
    public partial class GUI : Form
    {
        private int count=0;

        public GUI()
        {
            InitializeComponent();

            System.Timers.Timer aTimer = new System.Timers.Timer();
            aTimer.Elapsed += OnTimedEvent;
            aTimer.Interval = 10;
            aTimer.Enabled = true;
        }

        private void OnTimedEvent(object sender, System.Timers.ElapsedEventArgs e)
        {
            byte[] red  = { 255, 0, 0, 
                            255, 0, 0, 
                            255, 0, 0, 
                            255, 0, 0, 
                            255, 0, 0, 
                            255, 0, 0, 
                            255, 0, 0, 
                            255, 0, 0
                          };

            byte[] green ={ 0, 255, 0, 
                            0, 255, 0, 
                            0, 255, 0, 
                            0, 255, 0, 
                            0, 255, 0, 
                            0, 255, 0, 
                            0, 255, 0, 
                            0, 255, 0
                          };

            byte[] blue = { 0, 0, 255, 
                            0, 0, 255, 
                            0, 0, 255, 
                            0, 0, 255, 
                            0, 0, 255, 
                            0, 0, 255, 
                            0, 0, 255, 
                            0, 0, 255
                          };

            switch (count)
            {
                case 0: 
                    SendUdp(2000, "192.168.7.103", 1337, red);
                    count++;
                    break;

                case 1:
                    SendUdp(2000, "192.168.7.103", 1337, green);
                    count++;
                    break;

                case 2:
                    SendUdp(2000, "192.168.7.103", 1337, blue);
                    count = 0;
                    break;
            }     
        }      

        private void SendPacketButton_Click(object sender, EventArgs e)
        {
           
        }

        static void SendUdp(int srcPort, string dstIp, int dstPort, byte[] data)
        {
            using (UdpClient c = new UdpClient(srcPort))
                c.Send(data, data.Length, dstIp, dstPort);
        }
    }
}
