import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class DataProcessor {
    public static final int STRUCT_SIZE = 27; // 4+2+4+4+4+2+4+2+1 = 27 bytes (packed)
    
    public static class OutputData {
        public long current;
        public int voltage;
        public long power;
        public long thrust;
        public long torque;
        public int rpm;
        public long velocity;
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
    
    public static OutputData parseData(byte[] data) {
        if (data.length < STRUCT_SIZE) {
            throw new IllegalArgumentException("Dados insuficientes. Esperado: " + STRUCT_SIZE + ", Recebido: " + data.length);
        }
        
        ByteBuffer buffer = ByteBuffer.wrap(data);
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        
        OutputData output = new OutputData();
        
        output.current = Integer.toUnsignedLong(buffer.getInt());    // uint32_t
        output.voltage = Short.toUnsignedInt(buffer.getShort());     // uint16_t
        output.power = Integer.toUnsignedLong(buffer.getInt());      // uint32_t
        output.thrust = Integer.toUnsignedLong(buffer.getInt());     // uint32_t
        output.torque = Integer.toUnsignedLong(buffer.getInt());     // uint32_t
        output.rpm = Short.toUnsignedInt(buffer.getShort());         // uint16_t
        output.velocity = Integer.toUnsignedLong(buffer.getInt());   // uint32_t
        output.duty = Short.toUnsignedInt(buffer.getShort());        // uint16_t
        output.errTable = (short) (buffer.get() & 0xFF);             // uint8_t
        
        return output;
    }
    
    public static int findStructStart(byte[] data) {
        // Procura por uma struct válida (implementação básica)
        for (int i = 0; i <= data.length - STRUCT_SIZE; i++) {
            // Verifica se os próximos bytes formam uma struct plausível
            if (isPlausibleStruct(data, i)) {
                return i;
            }
        }
        return -1;
    }
    
    private static boolean isPlausibleStruct(byte[] data, int start) {
        // Verificações básicas de plausibilidade
        try {
            ByteBuffer buffer = ByteBuffer.wrap(data, start, STRUCT_SIZE);
            buffer.order(ByteOrder.LITTLE_ENDIAN);
            
            // Verifica ranges plausíveis
            long current = Integer.toUnsignedLong(buffer.getInt());
            int voltage = Short.toUnsignedInt(buffer.getShort());
            long power = Integer.toUnsignedLong(buffer.getInt());
            
            return current <= 100000 && voltage <= 50000 && power <= 5000000;
        } catch (Exception e) {
            return false;
        }
    }
}