import com.fazecast.jSerialComm.*;
import java.nio.charset.StandardCharsets;

public class SerialCommunication {
    private SerialPort serialPort;
    private boolean conectado = false;
    private String[] portasDisponiveis;
    private byte[] buffer = new byte[1024];
    private int bufferIndex = 0;
    private int totalPacotesRecebidos = 0;
    private StringBuilder bufferBrutos = new StringBuilder();
    private final Object lockBrutos = new Object();

    public SerialCommunication() {
        atualizarPortas();
    }

    public void atualizarPortas() {
        SerialPort[] ports = SerialPort.getCommPorts();
        portasDisponiveis = new String[ports.length];
        for (int i = 0; i < ports.length; i++) {
            portasDisponiveis[i] = ports[i].getSystemPortName() + " - " + ports[i].getDescriptivePortName();
        }
    }

    public String[] getPortasDisponiveis() {
        return portasDisponiveis;
    }

    public boolean conectar(String porta, int baudRate) {
        if (conectado) {
            return false;
        }

        String nomePorta = porta.split(" - ")[0];
        
        serialPort = SerialPort.getCommPort(nomePorta);
        serialPort.setBaudRate(baudRate);
        serialPort.setParity(SerialPort.NO_PARITY);
        serialPort.setNumStopBits(SerialPort.ONE_STOP_BIT);
        serialPort.setNumDataBits(8);
        serialPort.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 100, 0);

        if (serialPort.openPort()) {
            conectado = true;
            bufferIndex = 0;
            totalPacotesRecebidos = 0;
            System.out.println("✅ Conectado na porta: " + nomePorta + " | Baud rate: " + baudRate);
            return true;
        }
        System.out.println("❌ Falha ao conectar na porta: " + nomePorta);
        return false;
    }

    public void desconectar() {
        if (serialPort != null && conectado) {
            serialPort.closePort();
            conectado = false;
            System.out.println("🔌 Desconectado da porta serial");
        }
    }

    public boolean isConectado() {
        return conectado;
    }

    public DataProcessor.OutputData lerOutputData() {
        if (!conectado || serialPort == null) {
            return null;
        }
        
        try {
            int bytesAvailable = serialPort.bytesAvailable();
            if (bytesAvailable <= 0) {
                return null;
            }
            
            // Limita a leitura ao espaço disponível no buffer
            int bytesToRead = Math.min(bytesAvailable, buffer.length - bufferIndex);
            if (bytesToRead <= 0) {
                System.out.println("🔄 Buffer cheio, limpando...");
                bufferIndex = 0;
                bytesToRead = Math.min(bytesAvailable, buffer.length);
            }
            
            byte[] tempBuffer = new byte[bytesToRead];
            int bytesRead = serialPort.readBytes(tempBuffer, bytesToRead);
            
            if (bytesRead <= 0) {
                return null;
            }
            
            // ⭐⭐ ARMAZENA DADOS BRUTOS ⭐⭐
            synchronized(lockBrutos) {
                String dadosRaw = new String(tempBuffer, 0, bytesRead, StandardCharsets.US_ASCII);
                bufferBrutos.append(dadosRaw);
                
                // Limita o tamanho do buffer para não consumir muita memória
                if (bufferBrutos.length() > 10000) {
                    bufferBrutos.delete(0, 5000);
                }
            }
            
            System.out.println("📥 Bytes disponíveis: " + bytesAvailable + 
                             " | Lidos: " + bytesRead + 
                             " | Buffer index: " + bufferIndex);
            
            // Adiciona ao buffer
            System.arraycopy(tempBuffer, 0, buffer, bufferIndex, bytesRead);
            bufferIndex += bytesRead;
            
            // Converte o buffer para string para procurar por linhas completas
            String bufferStr = new String(buffer, 0, bufferIndex, StandardCharsets.US_ASCII);
            
            // Procura por linhas completas (terminadas com \r\n)
            int lineEnd = bufferStr.indexOf("\r\n");
            if (lineEnd != -1) {
                String line = bufferStr.substring(0, lineEnd);
                System.out.println("📨 Linha recebida: " + line);
                
                try {
                    DataProcessor.OutputData data = DataProcessor.parseStringFormat(line);
                    
                    totalPacotesRecebidos++;
                    // System.out.println("📊 PACOTE #" + totalPacotesRecebidos + " RECEBIDO:");
                    // System.out.println("   - Current: " + data.current);
                    // System.out.println("   - Voltage: " + data.voltage);
                    // System.out.println("   - Power: " + data.power);
                    // System.out.println("   - Thrust: " + data.thrust);
                    // System.out.println("   - Torque: " + data.torque);
                    // System.out.println("   - RPM: " + data.rpm);
                    // System.out.println("   - Velocity: " + data.velocity);
                    // System.out.println("   - Duty: " + data.duty);
                    // System.out.println("   - Err: " + data.errTable);
                    
                    // Remove a linha processada do buffer
                    int bytesToRemove = lineEnd + 2; // +2 para \r\n
                    int remaining = bufferIndex - bytesToRemove;
                    if (remaining > 0) {
                        System.arraycopy(buffer, bytesToRemove, buffer, 0, remaining);
                    }
                    bufferIndex = Math.max(0, remaining);
                    
                    return data;
                    
                } catch (Exception e) {
                    System.err.println("❌ Erro ao parsear linha: " + e.getMessage());
                    // Remove a linha problemática do buffer
                    int bytesToRemove = lineEnd + 2;
                    int remaining = bufferIndex - bytesToRemove;
                    if (remaining > 0) {
                        System.arraycopy(buffer, bytesToRemove, buffer, 0, remaining);
                    }
                    bufferIndex = Math.max(0, remaining);
                }
            }
            
            // Se o buffer está cheio e não encontrou linha completa, limpa
            if (bufferIndex >= buffer.length * 0.8) {
                System.out.println("🔄 Buffer 80% cheio sem linhas completas, limpando...");
                bufferIndex = 0;
            }
            
        } catch (Exception e) {
            System.err.println("❌ Erro na leitura serial: " + e.getMessage());
            bufferIndex = 0;
        }
        
        return null;
    }
    
    public boolean haDadosDisponiveis() {
        boolean disponivel = conectado && serialPort != null && serialPort.bytesAvailable() > 0;
        return disponivel;
    }

    public String getStatus() {
        if (conectado && serialPort != null) {
            return serialPort.getSystemPortName() + " | Pacotes: " + totalPacotesRecebidos;
        }
        return "Desconectado";
    }

    public boolean enviarDadosConfiguracao(InputData inputData) {
        if (!conectado || serialPort == null) {
            return false;
        }
        
        try {
            String comando = inputData.toSerialString();
            byte[] dados = comando.getBytes(StandardCharsets.US_ASCII);
            
            System.out.println("📤 ENVIANDO 10 VEZES:");
            System.out.println("   - Comando: " + comando.trim());
            System.out.println("   - Modo: " + inputData.getModeDescription());
            
            boolean algumSucesso = false;
            
            for(int i = 0; i < 10; i++) {
                int bytesEscritos = serialPort.writeBytes(dados, dados.length);
                boolean sucesso = (bytesEscritos == dados.length);
                
                if(sucesso) {
                    algumSucesso = true;
                    System.out.println("   ✅ Envio " + (i + 1) + ": OK");
                } else {
                    System.out.println("   ❌ Envio " + (i + 1) + ": FALHA");
                }
                
                // Delay de 50ms entre envios
                try {
                    Thread.sleep(50);
                } catch (InterruptedException ie) {
                    Thread.currentThread().interrupt();
                }
            }
            
            return algumSucesso;
            
        } catch (Exception e) {
            System.err.println("❌ Erro ao enviar dados: " + e.getMessage());
            return false;
        }
    }

    /**
     * Converte para formato string para envio (mais simples de debuggar)
     * Formato: "modo;thrustCal;torqueCal;duty\r\n"
     */

    public String getDadosBrutos() {
        synchronized(lockBrutos) {
            if (bufferBrutos.length() == 0) {
                return null;
            }
            String dados = bufferBrutos.toString();
            bufferBrutos.setLength(0); // Limpa o buffer após ler
            return dados;
        }
    }
    

}