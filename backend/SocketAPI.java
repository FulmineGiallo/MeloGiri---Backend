import java.io.*;
import java.net.*;

public class SocketAPI 
{
    String serverAddress;   // Indirizzo IP del Server
    int port;               // Porta del Server 
    Socket socket;

    SocketAPI(String serverAddress, int port)
    {
        this.serverAddress = serverAddress;
        this.port = port;
    }
    // TODO: Ritornare un Utente(model) cosi da poter utilizzarlo nell'applicazione
    void login(String email, String password)
    {
        String message = "1" + email + "&$" + password;
        System.out.println(createChannelSocket(message));
        
        
    }
    /*TODO: cambiare da String ad un array di Bevande della classe Bevanda(Model) */
    String getBevande()
    {
        try
        {
            socket =  new Socket(serverAddress, port);
            OutputStream output = socket.getOutputStream();
            PrintWriter writer = new PrintWriter(output, true);
            String message = "3";
            writer.println(message);
            InputStream inputStream = socket.getInputStream();
            byte[] buffer = new byte[100000];
            int bytesRead = inputStream.read(buffer);
            String receivedMessage = new String(buffer, 0, bytesRead);
            
            System.out.println("Received message from C socket server: " + receivedMessage);
            //Creo l'array con tutte le bevande

            // Close the socket
            socket.close();
        }
        catch (UnknownHostException ex)
        {
            System.err.println("Indirizzo IP del server non valido: " + serverAddress);
        } 
        catch (IOException ex) 
        {
            System.err.println("Errore di I/O durante la connessione al server: " + serverAddress);
        }
        
        return "ciao";
    }
    String register(String nome, String cognome, String data, String email, String password)
    {
        /* Il Client manda la sequenza: nome_cognome&data?email!password */
        String message = "2" + nome + "_" + cognome + "&" + data+ "?" + "email" + "!" + password;
        String response = createChannelSocket(message);
        System.out.println(response);
        return response;
    }

    /* 
        Questo metodo gestisce la connessione con la Socket 
        restituisce il messaggio che il Server manda al Client    
    */
    String createChannelSocket(String message)
    {
        String response = "Errore nella richiesta";

        try
        {
            socket =  new Socket(serverAddress, port);
            OutputStream output = socket.getOutputStream();
            PrintWriter writer = new PrintWriter(output, true);
            
            writer.println(message);
            InputStream inputStream = socket.getInputStream();
            System.out.println("Messaggio inviato al server: " + message);
            byte[] buffer = new byte[100000];
            int bytesRead = inputStream.read(buffer);
            String receivedMessage = new String(buffer, 0, bytesRead);
            response = receivedMessage;

            // Close the socket
            socket.close();
            
        }
        catch (UnknownHostException ex)
        {
            System.err.println("Indirizzo IP del server non valido: " + serverAddress);
        } 
        catch (IOException ex) 
        {
            System.err.println("Errore di I/O durante la connessione al server: " + serverAddress);
        }
        return response;
    }
}
