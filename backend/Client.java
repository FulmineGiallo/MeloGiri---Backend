import java.io.*;

public class Client 
{
    public static void main(String[] args) throws IOException 
    {
        SocketAPI richiesta = new SocketAPI("localhost", 8080);
        richiesta.login("carmine@email.com","1234prova");
    
    }
}
