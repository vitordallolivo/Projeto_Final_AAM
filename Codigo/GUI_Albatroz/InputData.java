import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class InputData {
    // Definições dos modos
    public static final byte NOTHING_MODE = 0x00;
    public static final byte CALIBRATION_MODE = (byte) 0xFF;
    public static final byte TARE_MODE = 0x0F;
    public static final byte MOTOR_MODE = (byte) 0xAF;
    
    private byte mode;
    private long calibrationFactorThrust;
    private long calibrationFactorTorque;
    private int duty;
    
    public InputData() {
        this.mode = NOTHING_MODE;
        this.calibrationFactorThrust = 0;
        this.calibrationFactorTorque = 0;
        this.duty = 0;
    }
    
    // Getters e Setters
    public byte getMode() { return mode; }
    public void setMode(byte mode) { this.mode = mode; }
    
    public long getCalibrationFactorThrust() { return calibrationFactorThrust; }
    public void setCalibrationFactorThrust(long calibrationFactorThrust) { 
        this.calibrationFactorThrust = calibrationFactorThrust; 
    }
    
    public long getCalibrationFactorTorque() { return calibrationFactorTorque; }
    public void setCalibrationFactorTorque(long calibrationFactorTorque) { 
        this.calibrationFactorTorque = calibrationFactorTorque; 
    }
    
    public int getDuty() { return duty; }
    public void setDuty(int duty) { 
        if (duty < 0) this.duty = 0;
        else if (duty > 100) this.duty = 100;
        else this.duty = duty;
    }
    
    /**
     * Converte a struct para array de bytes (formato C little-endian)
     */
    public byte[] toByteArray() {
        ByteBuffer buffer = ByteBuffer.allocate(11); // 1 + 4 + 4 + 2 = 11 bytes
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        
        buffer.put(mode);
        buffer.putInt((int) calibrationFactorThrust);
        buffer.putInt((int) calibrationFactorTorque);
        buffer.putShort((short) duty);
        
        return buffer.array();
    }
    
    /**
     * Retorna descrição do modo atual
     */
    public String getModeDescription() {
        switch (mode) {
            case NOTHING_MODE: return "NOTHING MODE";
            case CALIBRATION_MODE: return "CALIBRATION MODE";
            case TARE_MODE: return "TARE MODE";
            case MOTOR_MODE: return "MOTOR MODE";
            default: return "MODO DESCONHECIDO";
        }
    }
    
    @Override
    public String toString() {
        return String.format(
            "Mode: %s (0x%02X), ThrustCal: %d, TorqueCal: %d, Duty: %d%%",
            getModeDescription(), mode & 0xFF, calibrationFactorThrust, 
            calibrationFactorTorque, duty
        );
    }

    public String toSerialString() {
        return String.format("0x73.%d;%d;%d;%d.0x84\r\n", 
            mode & 0xFF,           // Converte para unsigned
            calibrationFactorThrust, 
            calibrationFactorTorque, 
            duty
        );
    }
}