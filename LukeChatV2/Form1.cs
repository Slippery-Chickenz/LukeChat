using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Net;
using System.Net.Sockets;

namespace LukeChatV2
{

    public partial class Form1 : Form
    {

        public delegate void InvokeDelegate();
        public delegate void InvokeDelegate2(String s);


        public Form1()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            Program.StartListening();
            String ListeningString = Program.localEndPoint.ToString();
            String PublicIPString = Program.getPublicIP();
            richTextBox1.Text = String.Format("Listening on local port: {0}\nPublic IP is: {1}\n",
                                ListeningString, PublicIPString);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            String connectionIP = "";
            connectionIP = textBox1.Text;
            IPEndPoint connectionEndPoint = new IPEndPoint(IPAddress.Parse(connectionIP), 27015);
            Program.ExecuteClient(connectionEndPoint);
        }

        public void ClientConnected()
        {
            this.BeginInvoke(new InvokeDelegate(InvokeMethodClientConnected));
        }
        public void InvokeMethodClientConnected()
        {
            richTextBox1.AppendText(String.Format("Client Connected in from: {0}\n"
                                , Program.reciever.RemoteEndPoint.ToString()));
        }

        public void DisplayMessage(String s)
        {
            this.BeginInvoke(new InvokeDelegate2(InvokeMethodDisplayMessage), new object[] {s});
        }
        public void InvokeMethodDisplayMessage(String s)
        {
            richTextBox2.AppendText(String.Format(s));
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void richTextBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {

        }

        public bool getLANBox()
        {
            return checkBox1.Checked;
        }

        private void button3_Click(object sender, EventArgs e)
        {

            String message = DateTime.Now.ToString("MM/dd/yyyy HH:mm:ss |::| ") + textBox2.Text;
            message += "\n";
            Program.SendMessage(message);
            textBox2.Clear();
        }

        private void textBox2_TextChanged(object sender, EventArgs e)
        {

        }
    }
}
