import com.fazecast.jSerialComm.*;
import java.util.Arrays;

public class SerialCommunication {
    private SerialPort serialPort;
    private boolean conectado = false;
    private String[] portasDisponiveis;
    private byte[] buffer = new byte[1024];
    private int bufferIndex = 0;

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
            bufferIndex = 0; // Reseta buffer
            return true;
        }
        return false;
    }

    public void desconectar() {
        if (serialPort != null && conectado) {
            serialPort.closePort();
            conectado = false;
        }
    }

    public boolean isConectado() {
        return conectado;
    }

    public DataProcessor.OutputData lerOutputData() {
        if (!conectado || serialPort == null) {
            return null;
        }
        
        int bytesAvailable = serialPort.bytesAvailable();
        if (bytesAvailable > 0) {
            byte[] tempBuffer = new byte[bytesAvailable];
            int bytesRead = serialPort.readBytes(tempBuffer, bytesAvailable);
            
            // Adiciona ao buffer
            if (bufferIndex + bytesRead > buffer.length) {
                // Buffer cheio, mantém apenas os últimos bytes
                int keep = Math.min(buffer.length - bytesRead, bufferIndex);
                System.arraycopy(buffer, bufferIndex - keep, buffer, 0, keep);
                bufferIndex = keep;
            }
            
            System.arraycopy(tempBuffer, 0, buffer, bufferIndex, bytesRead);
            bufferIndex += bytesRead;
            
            // Procura por struct válida
            int start = DataProcessor.findStructStart(Arrays.copyOf(buffer, bufferIndex));
            if (start >= 0 && start + DataProcessor.STRUCT_SIZE <= bufferIndex) {
                try {
                    DataProcessor.OutputData data = DataProcessor.parseData(
                        Arrays.copyOfRange(buffer, start, start + DataProcessor.STRUCT_SIZE)
                    );
                    
                    // Remove dados processados do buffer
                    int remaining = bufferIndex - (start + DataProcessor.STRUCT_SIZE);
                    if (remaining > 0) {
                        System.arraycopy(buffer, start + DataProcessor.STRUCT_SIZE, buffer, 0, remaining);
                    }
                    bufferIndex = remaining;
                    
                    return data;
                } catch (Exception e) {
                    System.err.println("Erro ao parsear dados: " + e.getMessage());
                    bufferIndex = 0; // Reset em caso de erro
                }
            }
        }
        return null;
    }
    
    public boolean haDadosDisponiveis() {
        return conectado && serialPort != null && serialPort.bytesAvailable() >= DataProcessor.STRUCT_SIZE;
    }

    public String getStatus() {
        if (conectado && serialPort != null) {
            return serialPort.getSystemPortName();
        }
        return "Desconectado";
    }

   public boolean enviarDadosConfiguracao(InputData inputData) {
    if (!conectado || serialPort == null) {
        return false;
    }
    
    try {
        byte[] dados = inputData.toByteArray();
        int bytesEscritos = serialPort.writeBytes(dados, dados.length);
        
        System.out.println("📤 Enviados " + bytesEscritos + " bytes: " + inputData.toString());
        
        // Log detalhado do que foi enviado
        System.out.println("📋 Detalhes do comando:");
        System.out.println("   - Modo: " + inputData.getModeDescription() + " (0x" + String.format("%02X", inputData.getMode()) + ")");
        System.out.println("   - Thrust Cal: " + inputData.getCalibrationFactorThrust());
        System.out.println("   - Torque Cal: " + inputData.getCalibrationFactorTorque());
        System.out.println("   - Duty: " + inputData.getDuty() + "%");
        
        return bytesEscritos == dados.length;
    } catch (Exception e) {
        System.err.println("❌ Erro ao enviar dados: " + e.getMessage());
        return false;
    }
}
}