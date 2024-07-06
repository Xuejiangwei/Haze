#pragma once

//https://www.codemag.com/article/1809051/Writing-Your-Own-Debugger-and-Language-Extensions-with-Visual-Studio-Code

class HazeVM;

class HazeDebuggerServer
{
public:
	static void InitDebuggerServer(HazeVM* m_VM);

	static void SendData(char* m_Data, int Length, int Flags = 0);

private:
	static void Start(HazeVM* m_VM);

	static void Recv();
};

//public static Action<Debugger, string> OnRequest;
//public static bool DebuggerAttached{ set; get; }
//
//static Debugger m_debugger
//static TcpClient m_client;
//static NetworkStream m_stream;
//
//static BlockingCollection<string> m_queue = new BlockingCollection<string>();
//
//public static void StartServer(int port = 13337)
//{
//    ThreadPool.QueueUserWorkItem(StartServerBlocked, port);
//}
//
//public static void StartServerBlocked(Object threadContext)
//{
//    int port = (int)threadContext;
//    IPAddress localAddr = IPAddress.Parse("127.0.0.1");
//    TcpListener server = new TcpListener(localAddr, port);
//    server.Start();
//    DebuggerAttached = true;
//
//    while (true) {
//        // Perform a blocking call to accept requests.
//        m_client = server.AcceptTcpClient();
//        m_stream = m_client.GetStream();
//        ThreadPool.QueueUserWorkItem(RunClient);
//    }
//}
//
//static void RunClient()
//{
//    Byte[] bytes = new Byte[1024];
//    string data = null;
//    int i;
//
//    ThreadPool.QueueUserWorkItem(StartProcessing, null);
//    m_debugger = new Debugger();
//    Debugger.OnResult += SendBack;
//
//    try {
//        while ((i = m_stream.Read(bytes, 0, bytes.Length)) != 0) {
//            data = System.Text.Encoding.UTF8.GetString(bytes, 0, i);
//            m_queue.Add(data);
//        }
//    }
//    catch (Exception exc) {
//        Console.Write("Client disconnected: {0}", exc.Message);
//    }
//    Debugger.OnResult -= SendBack;
//    m_client.Close();
//}
//
//static void StartProcessing(Object threadContext)
//{
//#if UNITY_EDITOR
//    // ProcessQueue will be called directly from Unity Update()
//# else
//    try {
//        ProcessQueue();
//    }
//    catch (Exception exc) {
//        Console.Write("Connection is over: {0}", exc.Message);
//    }
//#endif
//}
//
//public static void ProcessQueue()
//{
//    string data;
//#if UNITY_EDITOR
//    while (m_queue.TryTake(out data)) {
//#else
//    while (true)
//    {
//        // A blocking call
//        data = m_queue.Take();
//#endif
//        if (OnRequest != null) {
//            OnRequest ? .Invoke(m_debugger, data);
//        }
//        else {
//            m_debugger.ProcessClientCommands(data);
//        }
//    }
//    }
//static void SendBack(string str)
//{
//    byte[] msg = System.Text.Encoding.UTF8.GetBytes(str);
//    try {
//        m_stream.Write(msg, 0, msg.Length);
//        m_stream.Flush();
//    }
//    catch (Exception exc) {
//        Console.Write("Client disconnected: {0}", exc.Message);
//        return;
//    }
//}