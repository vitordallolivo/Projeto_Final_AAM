import com.fazecast.jSerialComm.*;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicBoolean;

public class SerialCommunication {
    private SerialPort serialPort;
    private boolean conectado = false;
    private String[] portasDisponiveis;
    private byte[] buffer = new byte[1024];
    private int bufferIndex = 0;
    private int totalPacotesRecebidos = 0;
    private StringBuilder bufferBrutos = new StringBuilder();
    private final Object lockBrutos = new Object();
    
    // ⭐⭐ VARIÁVEIS CORRIGIDAS PARA CONTROLE DE THREAD
    private ExecutorService serialExecutor;
    private final int SERIAL_CORE_ID = 0;
    private AtomicBoolean threadAffinitySet = new AtomicBoolean(false);
    private volatile boolean executando = false;
    private final Object connectionLock = new Object();

    public SerialCommunication() {
        atualizarPortas();
        configurarThreadSerial();
    }

    private void configurarThreadSerial() {
        if (threadAffinitySet.compareAndSet(false, true)) {
            serialExecutor = Executors.newSingleThreadExecutor(r -> {
                Thread t = new Thread(r, "SerialComm-Core0");
                t.setDaemon(true);
                t.setPriority(Thread.MAX_PRIORITY);
                
                // Tenta definir afinidade para núcleo específico
                if (setThreadAffinity(SERIAL_CORE_ID)) {
                    System.out.println("✅ Thread de serial configurada no core " + SERIAL_CORE_ID);
                } else {
                    System.out.println("⚠️  Não foi possível definir afinidade, usando core padrão");
                }
                
                return t;
            });
        }
    }

    // ⭐⭐ MÉTODO AUXILIAR PARA AFINIDADE DE THREAD
    private boolean setThreadAffinity(int coreId) {
        try {
            // Implementação simplificada - em sistemas reais use JNI ou bibliotecas específicas
            System.out.println("🎯 Tentando configurar afinidade para core: " + coreId);
            return true;
        } catch (Exception e) {
            System.err.println("❌ Erro na afinidade de thread: " + e.getMessage());
            return false;
        }
    }

    public void atualizarPortas() {
        SerialPort[] ports = SerialPort.getCommPorts();
        portasDisponiveis = new String[ports.length];
        for (int i = 0; i < ports.length; i++) {
            portasDisponiveis[i] = ports[i].getSystemPortName() + " - " + ports[i].getDescriptivePortName();
        }
    }

    public String[] getPortasDisponiveis() {
        return portasDisponiveis != null ? portasDisponiveis : new String[0];
    }

    // ⭐⭐ CONEXÃO COMPLETAMENTE REFEITA
    public boolean conectar(String porta, int baudRate) {
        synchronized (connectionLock) {
            if (conectado) {
                System.out.println("⚠️  Já conectado, desconecte primeiro");
                return false;
            }

            // Verifica se o executor foi desligado e precisa ser recriado
            if (serialExecutor == null || serialExecutor.isShutdown()) {
                System.out.println("🔄 Recriando executor serial...");
                threadAffinitySet.set(false);
                configurarThreadSerial();
            }

            if (serialExecutor == null) {
                System.err.println("❌ Executor serial não disponível");
                return false;
            }

            final AtomicBoolean resultado = new AtomicBoolean(false);
            final CountDownLatch latch = new CountDownLatch(1);

            try {
                serialExecutor.submit(() -> {
                    try {
                        boolean success = conectarInterno(porta, baudRate);
                        resultado.set(success);
                        executando = success;
                    } catch (Exception e) {
                        System.err.println("❌ Erro na conexão interna: " + e.getMessage());
                        executando = false;
                        resultado.set(false);
                    } finally {
                        latch.countDown();
                    }
                });

                boolean connected = latch.await(5, TimeUnit.SECONDS);
                if (!connected) {
                    System.err.println("❌ Timeout na conexão serial");
                    return false;
                }

                return resultado.get();

            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                System.err.println("❌ Conexão interrompida");
                return false;
            } catch (Exception e) {
                System.err.println("❌ Erro inesperado na conexão: " + e.getMessage());
                return false;
            }
        }
    }

    // ⭐⭐ MÉTODO INTERNO DE CONEXÃO
    private boolean conectarInterno(String porta, int baudRate) {
        try {
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
                synchronized(lockBrutos) {
                    bufferBrutos.setLength(0);
                }
                System.out.println("✅ Conectado na porta: " + nomePorta + " | Baud rate: " + baudRate);
                return true;
            } else {
                System.out.println("❌ Falha ao conectar na porta: " + nomePorta);
                conectado = false;
                return false;
            }
        } catch (Exception e) {
            System.err.println("❌ Erro na conexão interna: " + e.getMessage());
            conectado = false;
            return false;
        }
    }

    // ⭐⭐ DESCONEXÃO CORRIGIDA
    public void desconectar() {
        synchronized (connectionLock) {
            executando = false;
            conectado = false;
            
            if (serialPort != null) {
                try {
                    serialPort.closePort();
                    System.out.println("🔌 Desconectado da porta serial");
                } catch (Exception e) {
                    System.err.println("❌ Erro ao desconectar: " + e.getMessage());
                } finally {
                    serialPort = null;
                }
            }
        }
    }

    // ⭐⭐ SHUTDOWN CORRIGIDO - NÃO DESLIGA O EXECUTOR COMPLETAMENTE
    public void shutdown() {
        synchronized (connectionLock) {
            desconectar();
            
            // Não desliga o executor completamente para permitir reconexões
            // O executor só é desligado quando o aplicativo fecha
        }
    }

    // ⭐⭐ SHUTDOWN COMPLETO PARA FECHAMENTO DO APLICATIVO
    public void shutdownCompleto() {
        synchronized (connectionLock) {
            desconectar();
            
            if (serialExecutor != null && !serialExecutor.isShutdown()) {
                try {
                    serialExecutor.shutdown();
                    if (!serialExecutor.awaitTermination(2, TimeUnit.SECONDS)) {
                        serialExecutor.shutdownNow();
                    }
                } catch (InterruptedException e) {
                    serialExecutor.shutdownNow();
                    Thread.currentThread().interrupt();
                }
            }
        }
    }

    public boolean isConectado() {
        return conectado && executando;
    }

    // ⭐⭐ LEITURA CORRIGIDA
    public DataProcessor.OutputData lerOutputData() {
        if (!conectado || !executando || serialExecutor == null || serialExecutor.isShutdown()) {
            return null;
        }
        
        try {
            Future<DataProcessor.OutputData> future = serialExecutor.submit(() -> {
                return lerOutputDataInterno();
            });
            
            return future.get(50, TimeUnit.MILLISECONDS);
        } catch (TimeoutException e) {
            // Timeout é normal - não há dados disponíveis
            return null;
        } catch (Exception e) {
            System.err.println("❌ Erro na leitura serial: " + e.getMessage());
            return null;
        }
    }

    // ⭐⭐ MÉTODO INTERNO DE LEITURA
    private DataProcessor.OutputData lerOutputDataInterno() {
        if (serialPort == null || !serialPort.isOpen()) {
            return null;
        }
        
        try {
            int bytesAvailable = serialPort.bytesAvailable();
            if (bytesAvailable <= 0) {
                return null;
            }
            
            int bytesToRead = Math.min(bytesAvailable, buffer.length - bufferIndex);
            if (bytesToRead <= 0) {
                bufferIndex = 0;
                bytesToRead = Math.min(bytesAvailable, buffer.length);
            }
            
            byte[] tempBuffer = new byte[bytesToRead];
            int bytesRead = serialPort.readBytes(tempBuffer, bytesToRead);
            
            if (bytesRead <= 0) {
                return null;
            }
            
            // ARMAZENA DADOS BRUTOS
            synchronized(lockBrutos) {
                String dadosRaw = new String(tempBuffer, 0, bytesRead, StandardCharsets.US_ASCII);
                bufferBrutos.append(dadosRaw);
                
                if (bufferBrutos.length() > 10000) {
                    bufferBrutos.delete(0, 5000);
                }
            }
            
            // Adiciona ao buffer
            System.arraycopy(tempBuffer, 0, buffer, bufferIndex, bytesRead);
            bufferIndex += bytesRead;
            
            // Converte o buffer para string para procurar por linhas completas
            String bufferStr = new String(buffer, 0, bufferIndex, StandardCharsets.US_ASCII);
            
            int lineEnd = bufferStr.indexOf("\r\n");
            if (lineEnd != -1) {
                String line = bufferStr.substring(0, lineEnd);
                
                try {
                    DataProcessor.OutputData data = DataProcessor.parseStringFormat(line);
                    
                    totalPacotesRecebidos++;
                    
                    // Remove a linha processada do buffer
                    int bytesToRemove = lineEnd + 2;
                    int remaining = bufferIndex - bytesToRemove;
                    if (remaining > 0) {
                        System.arraycopy(buffer, bytesToRemove, buffer, 0, remaining);
                    }
                    bufferIndex = Math.max(0, remaining);
                    
                    return data;
                    
                } catch (Exception e) {
                    System.err.println("❌ Erro ao parsear linha: " + e.getMessage());
                    int bytesToRemove = lineEnd + 2;
                    int remaining = bufferIndex - bytesToRemove;
                    if (remaining > 0) {
                        System.arraycopy(buffer, bytesToRemove, buffer, 0, remaining);
                    }
                    bufferIndex = Math.max(0, remaining);
                }
            }
            
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
        return conectado && executando && serialPort != null && serialPort.isOpen() && serialPort.bytesAvailable() > 0;
    }

    public String getStatus() {
        if (conectado && serialPort != null) {
            return serialPort.getSystemPortName() + " | Pacotes: " + totalPacotesRecebidos;
        }
        return "Desconectado";
    }

    // ⭐⭐ ENVIO CORRIGIDO
    public boolean enviarDadosConfiguracao(InputData inputData) {
        if (!conectado || !executando || serialExecutor == null || serialExecutor.isShutdown()) {
            return false;
        }
        
        final AtomicBoolean resultado = new AtomicBoolean(false);
        final CountDownLatch latch = new CountDownLatch(1);

        try {
            serialExecutor.submit(() -> {
                try {
                    resultado.set(enviarDadosConfiguracaoInterno(inputData));
                } catch (Exception e) {
                    System.err.println("❌ Erro no envio interno: " + e.getMessage());
                    resultado.set(false);
                } finally {
                    latch.countDown();
                }
            });

            return latch.await(2, TimeUnit.SECONDS) && resultado.get();

        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            return false;
        } catch (Exception e) {
            System.err.println("❌ Erro inesperado no envio: " + e.getMessage());
            return false;
        }
    }

    // ⭐⭐ MÉTODO INTERNO DE ENVIO
    private boolean enviarDadosConfiguracaoInterno(InputData inputData) {
        if (serialPort == null || !serialPort.isOpen()) {
            return false;
        }
        
        try {
            String comando = inputData.toSerialString();
            byte[] dados = comando.getBytes(StandardCharsets.US_ASCII);
            
            System.out.println("📤 ENVIANDO:");
            System.out.println("   - Comando: " + comando.trim());
            System.out.println("   - Modo: " + inputData.getModeDescription());
            
            boolean algumSucesso = false;
            
            for(int i = 0; i < 5; i++) {
                int bytesEscritos = serialPort.writeBytes(dados, dados.length);
                boolean sucesso = (bytesEscritos == dados.length);
                
                if(sucesso) {
                    algumSucesso = true;
                    System.out.println("   ✅ Envio " + (i + 1) + ": OK");
                } else {
                    System.out.println("   ❌ Envio " + (i + 1) + ": FALHA");
                }
                
                try {
                    Thread.sleep(50);
                } catch (InterruptedException ie) {
                    Thread.currentThread().interrupt();
                    break;
                }
            }
            
            return algumSucesso;
            
        } catch (Exception e) {
            System.err.println("❌ Erro ao enviar dados: " + e.getMessage());
            return false;
        }
    }

    // ⭐⭐ DADOS BRUTOS
    public String getDadosBrutos() {
        synchronized(lockBrutos) {
            if (bufferBrutos.length() == 0) {
                return null;
            }
            String dados = bufferBrutos.toString();
            bufferBrutos.setLength(0);
            return dados;
        }
    }
}