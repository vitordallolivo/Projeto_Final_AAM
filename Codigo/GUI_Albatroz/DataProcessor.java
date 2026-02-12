import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class DataProcessor {
    public static final int STRUCT_SIZE = 27; // 4+2+4+4+4+2+4+2+1 = 27 bytes (packed)
    
    public static class OutputData {
        public int current;
        public int voltage;
        public int power;
        public int thrust;
        public int torque;
        public int rpm;
        public int velocity;
        public int duty;
        public short errTable;
        
        @Override
        public String toString() {
            return String.format(
                "Current: %d, Voltage: %d, Power: %d, Thrust: %d, Torque: %d, RPM: %d, Velocity: %d, Duty: %d, Err: %d",
                current, voltage, power, thrust, torque, rpm, velocity, duty, errTable
            );
        }
    }
    
    // Método para parsear o formato string "0;8416;0;-1;-1;0;0;0;103.\r\n"
    // No DataProcessor, substitua o método parseStringFormat por este:
    public static OutputData parseStringFormat(String dataString) {
        try {
            System.out.println("🔧 Processando string: '" + dataString + "'");
            
            // Remove .\r\n, \r\n, . e espaços no final
            dataString = dataString.trim();
            if (dataString.endsWith(".\r\n")) {
                dataString = dataString.substring(0, dataString.length() - 3);
            } else if (dataString.endsWith("\r\n")) {
                dataString = dataString.substring(0, dataString.length() - 2);
            } else if (dataString.endsWith(".")) {
                dataString = dataString.substring(0, dataString.length() - 1);
            }
            
            System.out.println("🔧 String após limpeza: '" + dataString + "'");
            
            String[] parts = dataString.split(";");
            
            System.out.println("🔧 Número de partes: " + parts.length);
            for (int i = 0; i < parts.length; i++) {
                System.out.println("   Parte " + i + ": '" + parts[i] + "'");
            }
            
            if (parts.length != 9) {
                throw new IllegalArgumentException("Número de campos inválido: " + parts.length + ", esperado: 9");
            }
            
            OutputData output = new OutputData();
            output.current = Integer.parseInt(parts[0].trim());
            output.voltage = Integer.parseInt(parts[1].trim());
            output.power = Integer.parseInt(parts[2].trim());
            output.thrust = Integer.parseInt(parts[3].trim());
            output.torque = Integer.parseInt(parts[4].trim());
            output.rpm = Integer.parseInt(parts[5].trim());
            output.velocity = Integer.parseInt(parts[6].trim());
            output.duty = Integer.parseInt(parts[7].trim());
            output.errTable = Short.parseShort(parts[8].trim());
            
            System.out.println("✅ Parse realizado com sucesso!");
            return output;
            
        } catch (Exception e) {
            System.err.println("❌ Erro detalhado no parse:");
            System.err.println("   - String: '" + dataString + "'");
            System.err.println("   - Mensagem: " + e.getMessage());
            e.printStackTrace();
            throw new IllegalArgumentException("Formato string inválido: " + dataString, e);
        }
    }
    // Método original para dados binários (mantido para compatibilidade)
    public static OutputData parseData(byte[] data) {
        if (data.length < STRUCT_SIZE) {
            throw new IllegalArgumentException("Dados insuficientes. Esperado: " + STRUCT_SIZE + ", Recebido: " + data.length);
        }
        
        ByteBuffer buffer = ByteBuffer.wrap(data);
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        
        OutputData output = new OutputData();
        
        output.current = (buffer.getInt())/1000;
        output.voltage = (buffer.getShort() & 0xFFFF)/1000;
        output.power = (buffer.getInt())    ;
        output.thrust = buffer.getInt();
        output.torque = buffer.getInt();
        output.rpm = buffer.getShort() & 0xFFFF;
        output.velocity = buffer.getInt();
        output.duty = buffer.getShort() & 0xFFFF;
        output.errTable = (short) (buffer.get() & 0xFF);
        
        return output;
    }
    
    public static int findStructStart(byte[] data) {
        for (int i = 0; i <= data.length - STRUCT_SIZE; i++) {
            if (isPlausibleStruct(data, i)) {
                return i;
            }
        }
        return -1;
    }
    
    public static boolean isPlausibleStruct(byte[] data, int start) {
        try {
            ByteBuffer buffer = ByteBuffer.wrap(data, start, STRUCT_SIZE);
            buffer.order(ByteOrder.LITTLE_ENDIAN);
            
            int current = buffer.getInt();
            int voltage = buffer.getShort() & 0xFFFF;
            int power = buffer.getInt();
            
            return current >= -1000 && current <= 100000 && 
                   voltage >= 0 && voltage <= 20000 && 
                   power >= -1000000 && power <= 1000000;
        } catch (Exception e) {
            return false;
        }
    }
}