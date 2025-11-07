import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.text.SimpleDateFormat;
import java.util.Date;

public class SerialGUIAlbatroz {
    private SerialCommunication serial;
    private JFrame frame;
    
    // Componentes de controle
    private JComboBox<String> portaComboBox;
    private JComboBox<Integer> baudComboBox;
    private JButton conectarBtn, desconectarBtn, atualizarBtn;
    private JButton iniciarBtn, pararBtn, salvarBtn, pararSalvarBtn,dadosBrutosBtn;
    private JButton tareBtn, calibrarBtn, motorBtn;
    private JTextArea logArea;
    private JLabel statusLabel, dadosLabel, modoLabel;
    
    // Gráficos para cada métrica
    private RealTimeChart graficoCurrent, graficoVoltage, graficoPower;
    private RealTimeChart graficoThrust, graficoTorque, graficoRPM;
    private RealTimeChart graficoVelocity, graficoDuty;
    private JLabel errLabel;
    
    // Coletores de dados
    private DataCollector collectorCurrent, collectorVoltage, collectorPower;
    private DataCollector collectorThrust, collectorTorque, collectorRPM;
    private DataCollector collectorVelocity, collectorDuty;
    
    private Timer timerGrafico;
    private boolean capturandoDados = false;
    private CSVWriter csvWriter;
    private boolean salvandoCSV = false;
    private SimpleDateFormat timeFormat = new SimpleDateFormat("HH:mm:ss.SSS");
    
    // Estado interno do modo de operação
    private byte modoAtual = InputData.NOTHING_MODE;
    private boolean motorLigado = false;
    private long calibracaoThrust = 1000;
    private long calibracaoTorque = 1000;
    
    // Variáveis para rastrear o último comando enviado
    private String ultimoComandoEnviado = "NONE";
    private String ultimoModoEnviado = "NOTHING_MODE";
    private long ultimoThrustCalEnviado = 0;
    private long ultimoTorqueCalEnviado = 0;
    private int ultimoDutyEnviado = 0;

    public SerialGUIAlbatroz() {
        serial = new SerialCommunication();
        inicializarCollectors();
        criarGUI();
        atualizarPortas();
        atualizarStatus();
        atualizarModoDisplay();
    }

    private void inicializarCollectors() {
        collectorCurrent = new DataCollector(200);
        collectorVoltage = new DataCollector(200);
        collectorPower = new DataCollector(200);
        collectorThrust = new DataCollector(200);
        collectorTorque = new DataCollector(200);
        collectorRPM = new DataCollector(200);
        collectorVelocity = new DataCollector(200);
        collectorDuty = new DataCollector(200);
    }

    private void criarGUI() {
        frame = new JFrame("Albatroz - Monitoramento e Controle Automático");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setSize(1300, 1000);
        frame.setLayout(new BorderLayout());

        // === PAINEL SUPERIOR - CONTROLES ===
        JPanel topPanel = new JPanel(new BorderLayout());
        
        // Linha 1: Controles de conexão
        JPanel conexaoPanel = new JPanel(new FlowLayout());
        
        conexaoPanel.add(new JLabel("Porta:"));
        portaComboBox = new JComboBox<>();
        portaComboBox.setPreferredSize(new Dimension(200, 25));
        conexaoPanel.add(portaComboBox);

        conexaoPanel.add(new JLabel("Baud:"));
        baudComboBox = new JComboBox<>(new Integer[]{9600, 115200, 57600, 38400, 19200});
        baudComboBox.setSelectedItem(115200);
        conexaoPanel.add(baudComboBox);

        conectarBtn = new JButton("🔌 Conectar");
        desconectarBtn = new JButton("🔌 Desconectar");
        atualizarBtn = new JButton("🔄 Atualizar");

        conexaoPanel.add(conectarBtn);
        conexaoPanel.add(desconectarBtn);
        conexaoPanel.add(atualizarBtn);

        // Display do modo atual
        modoLabel = new JLabel("MODO: NOTHING");
        modoLabel.setFont(new Font("SansSerif", Font.BOLD, 14));
        modoLabel.setBorder(BorderFactory.createEmptyBorder(0, 20, 0, 20));
        modoLabel.setOpaque(true);
        modoLabel.setBackground(Color.LIGHT_GRAY);
        modoLabel.setPreferredSize(new Dimension(200, 25));
        conexaoPanel.add(modoLabel);

        // Linha 2: Controles de operação
        JPanel operacaoPanel = new JPanel(new FlowLayout());
        
        tareBtn = new JButton("⚖️ Tare");
        calibrarBtn = new JButton("🎯 Calibrar");
        motorBtn = new JButton("🚀 Ligar Motor");
        
        operacaoPanel.add(tareBtn);
        operacaoPanel.add(calibrarBtn);
        operacaoPanel.add(motorBtn);

        // Linha 3: Controles de dados
        JPanel dadosControlPanel = new JPanel(new FlowLayout());
        
        iniciarBtn = new JButton("📊 Iniciar Captura");
        pararBtn = new JButton("⏸️ Parar Captura");
        salvarBtn = new JButton("💾 Iniciar CSV");
        pararSalvarBtn = new JButton("💾 Parar CSV");
        dadosBrutosBtn = new JButton("📡 Dados Brutos");
        
        dadosLabel = new JLabel("Aguardando dados...");
        dadosLabel.setFont(new Font("Monospaced", Font.PLAIN, 12));
        dadosLabel.setBorder(BorderFactory.createEmptyBorder(0, 20, 0, 20));

        dadosControlPanel.add(iniciarBtn);
        dadosControlPanel.add(pararBtn);
        dadosControlPanel.add(salvarBtn);
        dadosControlPanel.add(pararSalvarBtn);
        dadosControlPanel.add(dadosBrutosBtn);
        dadosControlPanel.add(dadosLabel);

        topPanel.add(conexaoPanel, BorderLayout.NORTH);
        topPanel.add(operacaoPanel, BorderLayout.CENTER);
        topPanel.add(dadosControlPanel, BorderLayout.SOUTH);

        // === PAINEL CENTRAL - GRÁFICOS ===
        JPanel graficosPanel = new JPanel(new GridLayout(3, 3, 5, 5));
        
        graficoCurrent = new RealTimeChart("Current ", "A");
        graficoVoltage = new RealTimeChart("Voltage V", "V");
        graficoPower = new RealTimeChart("Power W", "W");
        graficoThrust = new RealTimeChart("Thrust N", "N");
        graficoTorque = new RealTimeChart("Torque Nm", "Nm");
        graficoRPM = new RealTimeChart("RPM", "RPM");
        graficoVelocity = new RealTimeChart("Velocity m/s", "m/s");
        graficoDuty = new RealTimeChart("Duty Cycle (%)", "%");
        
        graficosPanel.add(graficoCurrent);
        graficosPanel.add(graficoVoltage);
        graficosPanel.add(graficoPower);
        graficosPanel.add(graficoThrust);
        graficosPanel.add(graficoTorque);
        graficosPanel.add(graficoRPM);
        graficosPanel.add(graficoVelocity);
        graficosPanel.add(graficoDuty);
        
        // Painel para Error Table
        JPanel errPanel = new JPanel(new BorderLayout());
        errPanel.setBorder(BorderFactory.createTitledBorder("Error Table"));
        errLabel = new JLabel("0", SwingConstants.CENTER);
        errLabel.setFont(new Font("Monospaced", Font.BOLD, 20));
        errLabel.setForeground(Color.BLACK);
        errLabel.setOpaque(true);
        errLabel.setBackground(Color.WHITE);
        errPanel.add(errLabel, BorderLayout.CENTER);
        graficosPanel.add(errPanel);

        // === ÁREA DE LOG ===
        logArea = new JTextArea(8, 80);
        logArea.setEditable(false);
        logArea.setFont(new Font("Monospaced", Font.PLAIN, 11));
        JScrollPane scrollPane = new JScrollPane(logArea);

        // === STATUS ===
        statusLabel = new JLabel("Status: Desconectado");
        statusLabel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
        statusLabel.setFont(new Font("SansSerif", Font.BOLD, 12));
        statusLabel.setOpaque(true);
        statusLabel.setBackground(Color.LIGHT_GRAY);

        // === LAYOUT PRINCIPAL ===
        frame.add(topPanel, BorderLayout.NORTH);
        frame.add(graficosPanel, BorderLayout.CENTER);
        frame.add(scrollPane, BorderLayout.SOUTH);
        frame.add(statusLabel, BorderLayout.SOUTH);

        configurarEventos();
        configurarTimer();
        atualizarBotoes();

        frame.setVisible(true);
    }

    private void configurarEventos() {
        conectarBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                conectarSerial();
            }
        });

        desconectarBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                desconectarSerial();
            }
        });

        atualizarBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                atualizarPortas();
            }
        });

        iniciarBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                iniciarCaptura();
            }
        });

        pararBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                pararCaptura();
            }
        });

        salvarBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                iniciarSalvarCSV();
            }
        });

        pararSalvarBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                pararSalvarCSV();
            }
        });
        
        // Novos eventos para operações automáticas
        tareBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                executarTare();
            }
        });

        calibrarBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                abrirDialogoCalibracao();
            }
        });

        motorBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                abrirDialogoMotor();
            }
        });
        dadosBrutosBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                abrirJanelaDadosBrutos();
            }
        });
    }

    private void configurarTimer() {
        timerGrafico = new Timer(50, new ActionListener() { // 20 Hz
            public void actionPerformed(ActionEvent e) {
                if (capturandoDados && serial.haDadosDisponiveis()) {
                    DataProcessor.OutputData data = serial.lerOutputData();
                    if (data != null) {
                        processarDados(data);
                    }
                }
            }
        });
    }

  private void processarDados(DataProcessor.OutputData data) {
        // Calcula velocity dividido por 10
        double velocityReal = data.velocity / 10.0;
        
        // Atualiza gráficos
        graficoCurrent.adicionarPonto(data.current);
        graficoVoltage.adicionarPonto(data.voltage);
        graficoPower.adicionarPonto(data.power);
        graficoThrust.adicionarPonto(data.thrust);
        graficoTorque.adicionarPonto(data.torque);
        graficoRPM.adicionarPonto(data.rpm);
        graficoVelocity.adicionarPonto(velocityReal);
        graficoDuty.adicionarPonto(data.duty);
        
        // Atualiza coletores
        collectorCurrent.addData(data.current);
        collectorVoltage.addData(data.voltage);
        collectorPower.addData(data.power);
        collectorThrust.addData(data.thrust);
        collectorTorque.addData(data.torque);
        collectorRPM.addData(data.rpm);
        collectorVelocity.addData(velocityReal);
        collectorDuty.addData(data.duty);
        
        // Atualiza label de dados
        dadosLabel.setText(String.format(
            "I:%6.2f A | V:%5.1f V | P:%7.1f W | Th:%5.1f N | Tr:%5.3f Nm | RPM:%5d | Vel:%6.2f m/s | Duty:%3d %% | Err:%2d",
            data.current / 1000.0,           // mA → A
            data.voltage / 1000.0,           // mV → V
            data.power / 1000.0,             // mW → W
            data.thrust / 100.0,             // cN → N (centiNewton para Newton)
            data.torque / 1000.0,            // mNm → Nm
            data.rpm,                        // RPM
            data.velocity / 100.0,           // cm/s → m/s
            data.duty,                       // %
            data.errTable                    // código
        ));
        
        // Atualiza Error Table com descrição dos erros
        String descricaoErro = interpretarErro(data.errTable);
        
        if (data.errTable != 0) {
            // MOSTRA A DESCRIÇÃO DO ERRO EM VEZ DO NÚMERO
            errLabel.setText(descricaoErro);
            errLabel.setForeground(Color.RED);
            errLabel.setBackground(new Color(255, 200, 200));
            errLabel.setToolTipText("Código: " + data.errTable + " - " + descricaoErro);
            
            // Log do erro
            adicionarLog("⚠️  " + descricaoErro + " (Código: " + data.errTable + ")");
        } else {
            errLabel.setText("SEM ERROS");
            errLabel.setForeground(Color.GREEN);
            errLabel.setBackground(Color.WHITE);
            errLabel.setToolTipText("Sistema operando normalmente");
        }
        
        // Salva no CSV se estiver ativo
        if (salvandoCSV && csvWriter != null && csvWriter.isAberto()) {
            String timestamp = timeFormat.format(new Date());
            csvWriter.escreverDados(timestamp, data, ultimoComandoEnviado, ultimoModoEnviado, 
                                ultimoThrustCalEnviado, ultimoTorqueCalEnviado, ultimoDutyEnviado);
        }
        
        
    }

    private void iniciarCaptura() {
        capturandoDados = true;
        timerGrafico.start();
        adicionarLog("📊 Iniciando captura de dados...");
        atualizarBotoes();
    }

    private void pararCaptura() {
        capturandoDados = false;
        timerGrafico.stop();
        adicionarLog("⏸️ Captura de dados parada");
        atualizarBotoes();
    }

    private void iniciarSalvarCSV() {
        csvWriter = new CSVWriter("albatroz_data");
        
        if (csvWriter.abrir()) {
            salvandoCSV = true;
            adicionarLog("💾 Iniciando salvamento em: " + csvWriter.getCaminhoArquivo());
            atualizarBotoes();
        } else {
            adicionarLog("❌ Erro ao criar arquivo CSV!");
        }
    }

    private void pararSalvarCSV() {
        if (csvWriter != null) {
            salvandoCSV = false;
            csvWriter.fechar();
            adicionarLog("💾 Salvamento finalizado: " + csvWriter.getCaminhoArquivo());
            csvWriter = null;
            atualizarBotoes();
        }
    }

    private void executarTare() {
        if (!serial.isConectado()) {
            JOptionPane.showMessageDialog(frame, "Conecte-se primeiro!", "Erro", JOptionPane.ERROR_MESSAGE);
            return;
        }
        
        InputData inputData = new InputData();
        inputData.setMode(InputData.TARE_MODE);
        inputData.setCalibrationFactorThrust(calibracaoThrust);
        inputData.setCalibrationFactorTorque(calibracaoTorque);
        inputData.setDuty(0);
        
        if (serial.enviarDadosConfiguracao(inputData)) {
            // Atualiza último comando enviado
            ultimoComandoEnviado = "TARE";
            ultimoModoEnviado = "TARE_MODE";
            ultimoThrustCalEnviado = calibracaoThrust;
            ultimoTorqueCalEnviado = calibracaoTorque;
            ultimoDutyEnviado = 0;
            
            modoAtual = InputData.TARE_MODE;
            adicionarLog("⚖️ Comando TARE enviado - Zerando sensores (Calibração mantida: Thrust=" + calibracaoThrust + ", Torque=" + calibracaoTorque + ")");
            atualizarModoDisplay();
            
            // Volta para NOTHING_MODE após 2 segundos
            Timer timerTare = new Timer(2000, new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    modoAtual = InputData.NOTHING_MODE;
                    atualizarModoDisplay();
                }
            });
            timerTare.setRepeats(false);
            timerTare.start();
            
        } else {
            adicionarLog("❌ Falha ao enviar comando TARE");
        }
    }

    private void abrirDialogoCalibracao() {
        if (!serial.isConectado()) {
            JOptionPane.showMessageDialog(frame, "Conecte-se primeiro!", "Erro", JOptionPane.ERROR_MESSAGE);
            return;
        }
        
        JDialog dialog = new JDialog(frame, "Calibração", true);
        dialog.setSize(400, 300);
        dialog.setLocationRelativeTo(frame);
        dialog.setLayout(new BorderLayout());

        JPanel panel = new JPanel(new GridLayout(4, 2, 10, 10));
        panel.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        
        panel.add(new JLabel("Fator Calibração Thrust:"));
        JTextField thrustField = new JTextField(String.valueOf(calibracaoThrust));
        panel.add(thrustField);
        
        panel.add(new JLabel("Fator Calibração Torque:"));
        JTextField torqueField = new JTextField(String.valueOf(calibracaoTorque));
        panel.add(torqueField);
        
        panel.add(new JLabel(""));
        panel.add(new JLabel(""));
        
        JButton enviarBtn = new JButton("🎯 Executar Calibração");
        JButton cancelarBtn = new JButton("❌ Cancelar");
        
        JPanel buttonPanel = new JPanel(new FlowLayout());
        buttonPanel.add(enviarBtn);
        buttonPanel.add(cancelarBtn);
        
        dialog.add(panel, BorderLayout.CENTER);
        dialog.add(buttonPanel, BorderLayout.SOUTH);
        
        enviarBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                try {
                    long thrustCal = Long.parseUnsignedLong(thrustField.getText().trim());
                    long torqueCal = Long.parseUnsignedLong(torqueField.getText().trim());
                    
                    // Atualiza os valores de calibração
                    calibracaoThrust = thrustCal;
                    calibracaoTorque = torqueCal;
                    
                    InputData inputData = new InputData();
                    inputData.setMode(InputData.CALIBRATION_MODE);
                    inputData.setCalibrationFactorThrust(calibracaoThrust);
                    inputData.setCalibrationFactorTorque(calibracaoTorque);
                    inputData.setDuty(0);
                    
                    if (serial.enviarDadosConfiguracao(inputData)) {
                        // Atualiza último comando enviado
                        ultimoComandoEnviado = "CALIBRACAO";
                        ultimoModoEnviado = "CALIBRATION_MODE";
                        ultimoThrustCalEnviado = calibracaoThrust;
                        ultimoTorqueCalEnviado = calibracaoTorque;
                        ultimoDutyEnviado = 0;
                        
                        modoAtual = InputData.CALIBRATION_MODE;
                        adicionarLog("🎯 Calibração atualizada - Thrust: " + calibracaoThrust + ", Torque: " + calibracaoTorque);
                        atualizarModoDisplay();
                        dialog.dispose();
                        
                        // Volta para NOTHING_MODE após 3 segundos
                        Timer timerCal = new Timer(3000, new ActionListener() {
                            public void actionPerformed(ActionEvent e) {
                                modoAtual = InputData.NOTHING_MODE;
                                atualizarModoDisplay();
                            }
                        });
                        timerCal.setRepeats(false);
                        timerCal.start();
                        
                    } else {
                        JOptionPane.showMessageDialog(dialog, "Falha no envio!", "Erro", JOptionPane.ERROR_MESSAGE);
                    }
                } catch (NumberFormatException ex) {
                    JOptionPane.showMessageDialog(dialog, "Valores inválidos! Use números inteiros.", "Erro", JOptionPane.ERROR_MESSAGE);
                }
            }
        });
        
        cancelarBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                dialog.dispose();
            }
        });
        
        dialog.setVisible(true);
    }


    private void abrirDialogoMotor() {
        if (!serial.isConectado()) {
            JOptionPane.showMessageDialog(frame, "Conecte-se primeiro!", "Erro", JOptionPane.ERROR_MESSAGE);
            return;
        }
        
        JDialog dialog = new JDialog(frame, "Controle do Motor", false);
        dialog.setSize(350, 250);
        dialog.setLocationRelativeTo(frame);
        dialog.setLayout(new BorderLayout());

        JPanel panel = new JPanel(new BorderLayout());
        panel.setBorder(BorderFactory.createEmptyBorder(20, 20, 20, 20));
        
        panel.add(new JLabel("Duty Cycle (%):", SwingConstants.CENTER), BorderLayout.NORTH);
        
        JSlider dutySlider = new JSlider(0, 100, ultimoDutyEnviado);
        JLabel dutyLabel = new JLabel(ultimoDutyEnviado + "%", SwingConstants.CENTER);
        dutyLabel.setFont(new Font("SansSerif", Font.BOLD, 16));
        
        dutySlider.addChangeListener(e -> {
            int value = dutySlider.getValue();
            dutyLabel.setText(value + "%");
        });
        
        panel.add(dutySlider, BorderLayout.CENTER);
        panel.add(dutyLabel, BorderLayout.SOUTH);
        
        JButton atualizarBtn = new JButton("🔄 Atualizar");
        JButton desligarBtn = new JButton("🛑 Desligar Motor");
        JButton fecharBtn = new JButton("❌ Fechar");
        
        JPanel buttonPanel = new JPanel(new FlowLayout());
        buttonPanel.add(atualizarBtn);
        buttonPanel.add(desligarBtn);
        buttonPanel.add(fecharBtn);
        
        dialog.add(panel, BorderLayout.CENTER);
        dialog.add(buttonPanel, BorderLayout.SOUTH);
        
        atualizarBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                int duty = dutySlider.getValue();
                ligarMotor(duty);
                // Não fecha o diálogo após atualizar
            }
        });
        
        desligarBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                desligarMotor();
                dutySlider.setValue(0);
                dutyLabel.setText("0%");
                // Não fecha o diálogo após desligar
            }
        });
        
        fecharBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                dialog.dispose();
            }
        });
        
        dialog.setVisible(true);
    }
    
    private void ligarMotor(int duty) {
        InputData inputData = new InputData();
        inputData.setMode(InputData.MOTOR_MODE);
        inputData.setCalibrationFactorThrust(calibracaoThrust);
        inputData.setCalibrationFactorTorque(calibracaoTorque);
        inputData.setDuty(duty);
        
        if (serial.enviarDadosConfiguracao(inputData)) {
            // Atualiza último comando enviado
            ultimoComandoEnviado = "MOTOR_LIGAR";
            ultimoModoEnviado = "MOTOR_MODE";
            ultimoThrustCalEnviado = calibracaoThrust;
            ultimoTorqueCalEnviado = calibracaoTorque;
            ultimoDutyEnviado = duty;
            
            modoAtual = InputData.MOTOR_MODE;
            motorLigado = true;
            adicionarLog("🚀 Motor ligado - Duty: " + duty + "% (Calibração: Thrust=" + calibracaoThrust + ", Torque=" + calibracaoTorque + ")");
            atualizarModoDisplay();
            atualizarBotoes();
        } else {
            adicionarLog("❌ Falha ao ligar motor");
        }
    }

    private void desligarMotor() {
        InputData inputData = new InputData();
        inputData.setMode(InputData.NOTHING_MODE);
        inputData.setCalibrationFactorThrust(calibracaoThrust);
        inputData.setCalibrationFactorTorque(calibracaoTorque);
        inputData.setDuty(0);
        
        if (serial.enviarDadosConfiguracao(inputData)) {
            // Atualiza último comando enviado
            ultimoComandoEnviado = "MOTOR_DESLIGAR";
            ultimoModoEnviado = "NOTHING_MODE";
            ultimoThrustCalEnviado = calibracaoThrust;
            ultimoTorqueCalEnviado = calibracaoTorque;
            ultimoDutyEnviado = 0;
            
            modoAtual = InputData.NOTHING_MODE;
            motorLigado = false;
            adicionarLog("🛑 Motor desligado (Calibração mantida: Thrust=" + calibracaoThrust + ", Torque=" + calibracaoTorque + ")");
            atualizarModoDisplay();
            atualizarBotoes();
        } else {
            adicionarLog("❌ Falha ao desligar motor");
        }
    }

    private void pararMotor() {
        InputData inputData = new InputData();
        inputData.setMode(InputData.NOTHING_MODE);
        inputData.setDuty(0);
        
        if (serial.enviarDadosConfiguracao(inputData)) {
            modoAtual = InputData.NOTHING_MODE;
            motorLigado = false;
            adicionarLog("🛑 Motor parado");
            atualizarModoDisplay();
            atualizarBotoes();
        } else {
            adicionarLog("❌ Falha ao parar motor");
        }
    }

    private void atualizarModoDisplay() {
        String texto = "";
        Color cor = Color.LIGHT_GRAY;
        
        switch (modoAtual) {
            case InputData.NOTHING_MODE:
                texto = "MODO: NOTHING";
                cor = Color.LIGHT_GRAY;
                break;
            case InputData.TARE_MODE:
                texto = "MODO: TARE";
                cor = Color.ORANGE;
                break;
            case InputData.CALIBRATION_MODE:
                texto = "MODO: CALIBRATION";
                cor = Color.BLUE;
                break;
            case InputData.MOTOR_MODE:
                texto = "MODO: MOTOR" + (motorLigado ? " (LIGADO)" : "");
                cor = motorLigado ? Color.RED : Color.PINK;
                break;
        }
        
        modoLabel.setText(texto);
        modoLabel.setBackground(cor);
        modoLabel.setForeground(cor == Color.LIGHT_GRAY ? Color.BLACK : Color.WHITE);
    }

    private void atualizarBotoes() {
        boolean conectado = serial.isConectado();
        
        conectarBtn.setEnabled(!conectado);
        desconectarBtn.setEnabled(conectado);
        portaComboBox.setEnabled(!conectado);
        baudComboBox.setEnabled(!conectado);
        
        tareBtn.setEnabled(conectado);
        calibrarBtn.setEnabled(conectado);
        motorBtn.setEnabled(conectado && !motorLigado);

        iniciarBtn.setEnabled(conectado && !capturandoDados);
        pararBtn.setEnabled(conectado && capturandoDados);
        salvarBtn.setEnabled(conectado && capturandoDados && !salvandoCSV);
        pararSalvarBtn.setEnabled(conectado && salvandoCSV);
    }

    private void atualizarPortas() {
        serial.atualizarPortas();
        String[] portas = serial.getPortasDisponiveis();
        
        portaComboBox.removeAllItems();
        for (String porta : portas) {
            portaComboBox.addItem(porta);
        }
        
        if (portas.length > 0) {
            adicionarLog("🔍 Portas atualizadas: " + portas.length + " encontrada(s)");
        } else {
            adicionarLog("❌ Nenhuma porta serial encontrada");
        }
    }

    private void conectarSerial() {
        if (portaComboBox.getSelectedItem() == null) {
            adicionarLog("❌ Selecione uma porta primeiro!");
            return;
        }

        String porta = (String) portaComboBox.getSelectedItem();
        int baudRate = (Integer) baudComboBox.getSelectedItem();

        adicionarLog("🔌 Tentando conectar em: " + porta + " (" + baudRate + " bauds)...");

        if (serial.conectar(porta, baudRate)) {
            adicionarLog("✅ Conectado com sucesso!");
            atualizarStatus();
        } else {
            adicionarLog("❌ Falha na conexão com " + porta);
        }
    }

    private void desconectarSerial() {
        // Para motor se estiver ligado
        if (motorLigado) {
            pararMotor();
        }
        
        pararCaptura();
        if (salvandoCSV) {
            pararSalvarCSV();
        }
        
        serial.desconectar();
        adicionarLog("🔌 Desconectado da porta serial");
        
        // Reseta estados
        modoAtual = InputData.NOTHING_MODE;
        motorLigado = false;
        atualizarModoDisplay();
        atualizarStatus();
        
        // Reseta a interface
        dadosLabel.setText("Aguardando dados...");
        errLabel.setText("0");
        errLabel.setForeground(Color.BLACK);
        errLabel.setBackground(Color.WHITE);
    }

    private void atualizarStatus() {
        String status = serial.isConectado() ? 
            "CONECTADO - " + serial.getStatus() : "DESCONECTADO";
        statusLabel.setText("Status: " + status);
        
        if (serial.isConectado()) {
            statusLabel.setBackground(Color.GREEN);
            statusLabel.setForeground(Color.BLACK);
        } else {
            statusLabel.setBackground(Color.LIGHT_GRAY);
            statusLabel.setForeground(Color.BLACK);
        }
        
        atualizarBotoes();
    }

    private void adicionarLog(String mensagem) {
        String timestamp = new SimpleDateFormat("HH:mm:ss").format(new Date());
        logArea.append("[" + timestamp + "] " + mensagem + "\n");
        logArea.setCaretPosition(logArea.getDocument().getLength());
    }

    public static void main(String[] args) {
        // Garante que a GUI seja criada na EDT do Swing
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                new SerialGUIAlbatroz();
            }
        });
    }

    private String interpretarErro(short errCode) {
        if (errCode == 0) return "SEM ERROS";
        
        StringBuilder erros = new StringBuilder();
        
        // Verifica cada bit e adiciona a descrição correspondente
        if ((errCode & (1 << 0)) != 0) erros.append("SEM TENSÃO");
        if ((errCode & (1 << 1)) != 0) erros.append(erros.length() > 0 ? " | SEM CORRENTE" : "SEM CORRENTE");
        if ((errCode & (1 << 2)) != 0) erros.append(erros.length() > 0 ? " | FALHA RC" : "FALHA RC");
        if ((errCode & (1 << 3)) != 0) erros.append(erros.length() > 0 ? " | PITOT SOLTO" : "PITOT SOLTO");
        if ((errCode & (1 << 4)) != 0) erros.append(erros.length() > 0 ? " | SEM RPM" : "SEM RPM");
        if ((errCode & (1 << 5)) != 0) erros.append(erros.length() > 0 ? " | CÉLULA DESCONECT." : "CÉLULA DESCONECT.");
        if ((errCode & (1 << 6)) != 0) erros.append(erros.length() > 0 ? " | CÉLULA NÃO CALIB." : "CÉLULA NÃO CALIB.");
        
        
        return erros.toString();
    }

    private void abrirJanelaDadosBrutos() {
        if (!serial.isConectado()) {
            JOptionPane.showMessageDialog(frame, "Conecte-se primeiro!", "Erro", JOptionPane.ERROR_MESSAGE);
            return;
        }
        
        JFrame janelaBrutos = new JFrame("Dados Brutos - USART");
        janelaBrutos.setSize(700, 500);
        janelaBrutos.setLocationRelativeTo(frame);
        janelaBrutos.setLayout(new BorderLayout());
        
        // Área de texto para mostrar dados brutos
        JTextArea areaBrutos = new JTextArea();
        areaBrutos.setFont(new Font("Monospaced", Font.PLAIN, 12));
        areaBrutos.setEditable(false);
        areaBrutos.setBackground(Color.WHITE);
        areaBrutos.setForeground(Color.BLACK);
        
        JScrollPane scrollPane = new JScrollPane(areaBrutos);
        
        // Painel de controles
        JPanel painelControles = new JPanel(new FlowLayout());
        
        JButton limparBtn = new JButton("🧹 Limpar");
        JButton pausarBtn = new JButton("⏸️ Pausar");
        JButton hexBtn = new JButton("🔢 Modo Hex");
        JCheckBox autoScroll = new JCheckBox("Auto-scroll", true);
        JButton salvarBtn = new JButton("💾 Salvar");
        
        painelControles.add(limparBtn);
        painelControles.add(pausarBtn);
        painelControles.add(hexBtn);
        painelControles.add(autoScroll);
        painelControles.add(salvarBtn);
        
        // Status
        JLabel statusBrutos = new JLabel("Capturando dados brutos...");
        statusBrutos.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
        
        janelaBrutos.add(scrollPane, BorderLayout.CENTER);
        janelaBrutos.add(painelControles, BorderLayout.NORTH);
        janelaBrutos.add(statusBrutos, BorderLayout.SOUTH);
        
        // Variáveis de controle
        final boolean[] pausado = {false};
        final boolean[] modoHex = {false};
        final int[] contadorBytes = {0};
        
        // Timer para capturar dados brutos - CORRIGIDO
        Timer timerBrutos = new Timer(100, new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (!pausado[0]) {
                    String dadosRaw = serial.getDadosBrutos();
                    if (dadosRaw != null && !dadosRaw.isEmpty()) {
                        contadorBytes[0] += dadosRaw.length();
                        
                        // ⭐⭐ APENAS OS DADOS BRUTOS - SEM LOGS ⭐⭐
                        if (modoHex[0]) {
                            // Converte para hex
                            StringBuilder hex = new StringBuilder();
                            byte[] bytes = dadosRaw.getBytes(StandardCharsets.US_ASCII);
                            for (int i = 0; i < bytes.length; i++) {
                                hex.append(String.format("%02X ", bytes[i]));
                                // Quebra de linha a cada 16 bytes
                                if ((i + 1) % 16 == 0) {
                                    hex.append("\n");
                                }
                            }
                            areaBrutos.append(hex.toString() + "\n");
                        } else {
                            // Mostra como texto puro - SEM adicionar "[TXT]" ou outros textos
                            areaBrutos.append(dadosRaw);
                        }
                        
                        statusBrutos.setText("Bytes totais: " + contadorBytes[0] + " | Última atualização: " + new SimpleDateFormat("HH:mm:ss").format(new Date()));
                        
                        if (autoScroll.isSelected()) {
                            areaBrutos.setCaretPosition(areaBrutos.getDocument().getLength());
                        }
                    } else {
                        statusBrutos.setText("Aguardando dados... | Bytes totais: " + contadorBytes[0]);
                    }
                }
            }
        });
        
        // Eventos dos botões (mantenha igual)
        limparBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                areaBrutos.setText("");
                contadorBytes[0] = 0;
                statusBrutos.setText("Buffer limpo");
            }
        });
        
        pausarBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                pausado[0] = !pausado[0];
                pausarBtn.setText(pausado[0] ? "▶️ Retomar" : "⏸️ Pausar");
                statusBrutos.setText(pausado[0] ? "PAUSADO" : "Capturando...");
            }
        });
        
        hexBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                modoHex[0] = !modoHex[0];
                hexBtn.setText(modoHex[0] ? "🔤 Modo Texto" : "🔢 Modo Hex");
                // Limpa ao mudar de modo para evitar mistura
                areaBrutos.setText("");
            }
        });
        
        salvarBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                try {
                    JFileChooser fileChooser = new JFileChooser();
                    fileChooser.setDialogTitle("Salvar dados brutos");
                    if (fileChooser.showSaveDialog(janelaBrutos) == JFileChooser.APPROVE_OPTION) {
                        File file = fileChooser.getSelectedFile();
                        try (PrintWriter writer = new PrintWriter(file)) {
                            writer.write(areaBrutos.getText());
                            JOptionPane.showMessageDialog(janelaBrutos, "Dados salvos com sucesso!", "Sucesso", JOptionPane.INFORMATION_MESSAGE);
                        }
                    }
                } catch (Exception ex) {
                    JOptionPane.showMessageDialog(janelaBrutos, "Erro ao salvar: " + ex.getMessage(), "Erro", JOptionPane.ERROR_MESSAGE);
                }
            }
        });
        
        // Quando fechar a janela, para o timer
        janelaBrutos.addWindowListener(new java.awt.event.WindowAdapter() {
            @Override
            public void windowClosing(java.awt.event.WindowEvent windowEvent) {
                timerBrutos.stop();
            }
        });
        
        // Mensagem inicial limpa
        areaBrutos.setText("=== Iniciando captura de dados brutos ===\n");
        areaBrutos.append("Conectado em: " + serial.getStatus() + "\n");
        areaBrutos.append("==========================================\n\n");
        
        timerBrutos.start();
        janelaBrutos.setVisible(true);
    }


}