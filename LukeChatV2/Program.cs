using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Runtime.InteropServices;

namespace LukeChatV2
{
    public class StateObject
    {
        public Socket workSocket = null;
        public const int BUFFER_SIZE = 1024;
        public byte[] buffer = new byte[BUFFER_SIZE];
        public StringBuilder sb = new StringBuilder();
    }

    public static class Program
    {
        // Establish the remote endpoint
        // for the socket. This example
        // uses port 27015 on the local computer

        public static IPHostEntry ipHost = Dns.GetHostEntry(Dns.GetHostName());
        public static IPAddress ipAddr = IPAddress.IPv6Any; //ipHost.AddressList[0]; // This is just an IP address
        public static IPEndPoint localEndPoint = new IPEndPoint(ipAddr, 27015); // But this is an IP address and a port

        // Creation TCP/IP Socket using
        // Socket Class Constructor
        public static Socket client = new Socket(ipAddr.AddressFamily,
                     SocketType.Stream, ProtocolType.Tcp);

        // Creation TCP/IP Socket using
        // Socket Class Constructor
        public static Socket sender = new Socket(ipAddr.AddressFamily,
                   SocketType.Stream, ProtocolType.Tcp);

        // Creation TCP/IP Socket using
        // Socket Class Constructor
        public static Socket listener = new Socket(ipAddr.AddressFamily,
                     SocketType.Stream, ProtocolType.Tcp);

        // Creation TCP/IP Socket using
        // Socket Class Constructor
        public static Socket reciever = new Socket(ipAddr.AddressFamily,
                    SocketType.Stream, ProtocolType.Tcp);

        // Booleans to tell if we are already connected/listening
        public static bool isConnected = false;
        public static bool isListening = false;

        // Object to refernce for the form
        public static Form1 MainForm = null;

        // Making Delegates to edit the GUI
        public delegate void PrintDelegate1(string myArg1);

        // Main Method
        [STAThread]
        static void Main()
        {
            listener.SetSocketOption(SocketOptionLevel.IPv6, SocketOptionName.IPv6Only, 0);

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            MainForm = new Form1();
            Application.Run(MainForm);

            NATUPNPLib.IUPnPNAT upnpnat = new NATUPNPLib.UPnPNAT();
            NATUPNPLib.IStaticPortMappingCollection mappings = upnpnat.StaticPortMappingCollection;

            IPHostEntry host = Dns.GetHostEntry(Dns.GetHostName());
            String IPv4LocalAddr = "";
            foreach (var ip in host.AddressList)
            {
                if (ip.AddressFamily == AddressFamily.InterNetwork)
                {
                    IPv4LocalAddr = ip.ToString();
                }
            }
            // Console.WriteLine(IPv4LocalAddr);

            /* mappings.Add(27015, "TCP", 27015, IPv4LocalAddr, true, "Test UPnP");

            foreach (NATUPNPLib.IStaticPortMapping portMapping in mappings)
            {
                Console.WriteLine(String.Format("Description: {0}", (string)portMapping.Description));
                Console.WriteLine(String.Format("Enabled: {0}", portMapping.Enabled));
                Console.WriteLine(String.Format("External IP: {0}", portMapping.ExternalIPAddress));
                Console.WriteLine(String.Format("Port: {0}", portMapping.ExternalPort));
                Console.WriteLine(String.Format("Internal Client: {0}", portMapping.InternalClient));
                Console.WriteLine(String.Format("Internal Port: {0}", portMapping.InternalPort));
                Console.WriteLine(String.Format("Protocol: {0}", portMapping.Protocol));
            }
            */

        }

        // Function to get the public IP
        public static String getPublicIP()
        {
            string externalIpString = new WebClient().DownloadString("http://icanhazip.com").Replace("\\r\\n", "").Replace("\\n", "").Trim();
            var externalIp = IPAddress.Parse(externalIpString);

            return externalIp.ToString();
        }


        // Function to start listening for connection requests on the listening sockets
        public static void StartListening()
        {

            // Bind the socket to the local endpoint and listen for incoming connections.  
            try
            {
                listener.Bind(localEndPoint);
                listener.Listen(100);
                isListening = true;

                // Start an asynchronous socket to listen for connections.
                listener.BeginAccept(new AsyncCallback(AcceptCallback), listener); // Start asynchronously accepting connections

            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }

        }

        // Function to call to Asynchronously Accept incomming connection requests
        public static void AcceptCallback(IAsyncResult ar)
        {
            // Get the socket that handles the client request.  
            reciever = listener.EndAccept(ar); // Accept the connection with the reciever socket
            MainForm.ClientConnected();


            if (!isConnected)
            {
                IPEndPoint remoteIpEndPoint = reciever.RemoteEndPoint as IPEndPoint;
                //ExecuteClient(remoteIpEndPoint);
            }


            StateObject so2 = new StateObject(); // Make a new State object to receive with
            so2.workSocket = reciever; // Assign the work socket to the state object
            reciever.BeginReceive(so2.buffer, 0, StateObject.BUFFER_SIZE, 0, // Calling BeginReceive will start asynchronously 
                                  new AsyncCallback(Read_Callback), so2); // receiving data from the connection
            // Once data is collected we call the Read_Callback command.


        }

        // Function to call in the Async Begin Receive call
        public static void Read_Callback(IAsyncResult ar)
        {
            StateObject so = (StateObject)ar.AsyncState; // Get the state object from the BeginReceive call
            Socket s = so.workSocket; // Get the socket that BeginReceive was called on

            int read = s.EndReceive(ar); // End the Receive from the socket
            // Make sure we read in something
            if (read > 0)
            {
                MainForm.DisplayMessage(Encoding.ASCII.GetString(so.buffer, 0, read));
                reciever.BeginReceive(so.buffer, 0, StateObject.BUFFER_SIZE, 0,
                                         new AsyncCallback(Read_Callback), so); // Begin receive again.
            }
        }

        // ExecuteClient() Method
        public static void ExecuteClient(IPEndPoint connectEndPoint)
        {
            if (isConnected) { return; }

            try
            {
                // Connect Socket to the remote
                // endpoint using method Connect()
                client.BeginConnect(connectEndPoint, new AsyncCallback(ConnectCallback), client);

            }

            // Manage of Socket's Exceptions
            catch (ArgumentNullException ane)
            {

                Console.WriteLine("ArgumentNullException : {0}", ane.ToString());
            }

            catch (SocketException se)
            {

                Console.WriteLine("SocketException : {0}", se.ToString());
            }

            catch (Exception e)
            {
                Console.WriteLine("Unexpected exception : {0}", e.ToString());
            }

        }

        public static void ConnectCallback(IAsyncResult ar)
        {
            sender = (Socket)ar.AsyncState;
            sender.EndConnect(ar);
            isConnected = true;

            // We print EndPoint information
            // that we are connected

            String TestMessage = String.Format("Connected to: {0}\n", sender.RemoteEndPoint.ToString());
            byte[] TestMessageASCII = ASCIIEncoding.ASCII.GetBytes(TestMessage);
            foreach (byte b in TestMessageASCII)
            {
                byte[] sent_byte = { b };
                sender.BeginSend(sent_byte, 0, sent_byte.Length, 0, new AsyncCallback(SendCallback), sender);
            }


        }

        public static void SendCallback(IAsyncResult ar)
        {
            int bytes_sent = sender.EndSend(ar);
            if (bytes_sent == 0)
            {
                Console.WriteLine("Character Failed to send.");
            }

        }

        public static void SendMessage(String message)
        {
            byte[] TestMessageASCII = ASCIIEncoding.ASCII.GetBytes(message);
            foreach (byte b in TestMessageASCII)
            {
                byte[] sent_byte = { b };
                sender.BeginSend(sent_byte, 0, sent_byte.Length, 0, new AsyncCallback(SendCallback), sender);
            }
        }

    }
}
