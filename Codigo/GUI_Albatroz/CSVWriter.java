import java.io.*;
import java.text.SimpleDateFormat;
import java.util.Date;

public class CSVWriter {
    private String arquivoPath;
    private PrintWriter writer;
    private boolean cabecalhoEscrito = false;

    public CSVWriter(String nomeBase) {
        String timestamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
        this.arquivoPath = nomeBase + "_" + timestamp + ".csv";
    }

    public boolean abrir() {
        try {
            writer = new PrintWriter(new FileWriter(arquivoPath, true));
            return true;
        } catch (IOException e) {
            System.err.println("Erro ao criar arquivo: " + e.getMessage());
            return false;
        }
    }

    public void escreverCabecalho() {
        if (!cabecalhoEscrito && writer != null) {
            writer.println("Timestamp,Current,Voltage,Power,Thrust,Torque,RPM,Velocity,Duty,ErrTable,Comando_Enviado,Modo_Enviado,ThrustCal_Enviado,TorqueCal_Enviado,Duty_Enviado");
            cabecalhoEscrito = true;
        }
    }

    public void escreverDados(String timestamp, DataProcessor.OutputData data, String comandoEnviado, String modoEnviado, long thrustCalEnviado, long torqueCalEnviado, int dutyEnviado) {
        if (writer != null) {
            if (!cabecalhoEscrito) {
                escreverCabecalho();
            }
            
            String linha = String.format("%s,%d,%d,%d,%d,%d,%d,%.1f,%d,%d,%s,%s,%d,%d,%d",
                timestamp,
                data.current, data.voltage, data.power, data.thrust, data.torque,
                data.rpm, data.velocity / 10.0, data.duty, data.errTable,
                comandoEnviado, modoEnviado, thrustCalEnviado, torqueCalEnviado, dutyEnviado
            );
            writer.println(linha);
            writer.flush();
        }
    }

    public void fechar() {
        if (writer != null) {
            writer.close();
        }
    }

    public String getCaminhoArquivo() {
        return arquivoPath;
    }

    public boolean isAberto() {
        return writer != null;
    }
}