import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class ConfigPanel extends JPanel {
    private SerialCommunication serial;
    private JTextArea logArea;
    
    // Componentes do painel
    private JComboBox<String> modeComboBox;
    private JTextField thrustCalField;
    private JTextField torqueCalField;
    private JSlider dutySlider;
    private JLabel dutyValueLabel;
    private JButton enviarBtn;
    private JButton limparBtn;
    
    public ConfigPanel(SerialCommunication serial, JTextArea logArea) {
        this.serial = serial;
        this.logArea = logArea;
        
        setLayout(new BorderLayout());
        setBorder(BorderFactory.createTitledBorder("Configuração do Dispositivo"));
        
        criarInterface();
    }
    
    private void criarInterface() {
        JPanel mainPanel = new JPanel(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.insets = new Insets(5, 5, 5, 5);
        gbc.fill = GridBagConstraints.HORIZONTAL;
        gbc.weightx = 1.0;
        
        // === MODO ===
        gbc.gridx = 0; gbc.gridy = 0;
        mainPanel.add(new JLabel("Modo de Operação:"), gbc);
        
        gbc.gridx = 1;
        modeComboBox = new JComboBox<>(new String[]{
            "NOTHING MODE", 
            "CALIBRATION MODE", 
            "TARE MODE", 
            "MOTOR MODE"
        });
        mainPanel.add(modeComboBox, gbc);
        
        // === CALIBRAÇÃO THRUST ===
        gbc.gridx = 0; gbc.gridy = 1;
        mainPanel.add(new JLabel("Fator Calibração Thrust:"), gbc);
        
        gbc.gridx = 1;
        thrustCalField = new JTextField("0");
        thrustCalField.setToolTipText("Valor uint32_t (0 a 4294967295)");
        mainPanel.add(thrustCalField, gbc);
        
        // === CALIBRAÇÃO TORQUE ===
        gbc.gridx = 0; gbc.gridy = 2;
        mainPanel.add(new JLabel("Fator Calibração Torque:"), gbc);
        
        gbc.gridx = 1;
        torqueCalField = new JTextField("0");
        torqueCalField.setToolTipText("Valor uint32_t (0 a 4294967295)");
        mainPanel.add(torqueCalField, gbc);
        
        // === DUTY CYCLE ===
        gbc.gridx = 0; gbc.gridy = 3;
        mainPanel.add(new JLabel("Duty Cycle (%):"), gbc);
        
        gbc.gridx = 1;
        JPanel dutyPanel = new JPanel(new BorderLayout());
        dutySlider = new JSlider(0, 100, 0);
        dutyValueLabel = new JLabel("0%", SwingConstants.CENTER);
        dutyValueLabel.setPreferredSize(new Dimension(40, 20));
        
        dutySlider.addChangeListener(e -> {
            int value = dutySlider.getValue();
            dutyValueLabel.setText(value + "%");
        });
        
        dutyPanel.add(dutySlider, BorderLayout.CENTER);
        dutyPanel.add(dutyValueLabel, BorderLayout.EAST);
        mainPanel.add(dutyPanel, gbc);
        
        // === BOTÕES ===
        gbc.gridx = 0; gbc.gridy = 4; gbc.gridwidth = 2;
        JPanel buttonPanel = new JPanel(new FlowLayout());
        
        enviarBtn = new JButton("Enviar Configuração");
        limparBtn = new JButton("Limpar Campos");
        
        buttonPanel.add(enviarBtn);
        buttonPanel.add(limparBtn);
        
        mainPanel.add(buttonPanel, gbc);
        
        // === ÁREA DE STATUS ===
        gbc.gridx = 0; gbc.gridy = 5; gbc.gridwidth = 2;
        JTextArea statusArea = new JTextArea(3, 30);
        statusArea.setEditable(false);
        statusArea.setBorder(BorderFactory.createTitledBorder("Status do Envio"));
        statusArea.setText("Preencha os campos e clique em 'Enviar Configuração'");
        mainPanel.add(new JScrollPane(statusArea), gbc);
        
        add(mainPanel, BorderLayout.CENTER);
        
        configurarEventos(statusArea);
        criarBotoesPredefinidos();
    }
    
    private void configurarEventos(JTextArea statusArea) {
        enviarBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                enviarConfiguracao(statusArea);
            }
        });
        
        limparBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                limparCampos();
                statusArea.setText("Campos limpos. Preencha e envie nova configuração.");
            }
        });
        
        // Enter nos campos de texto também envia
        ActionListener enterListener = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                enviarConfiguracao(statusArea);
            }
        };
        
        thrustCalField.addActionListener(enterListener);
        torqueCalField.addActionListener(enterListener);
    }
    
    private void criarBotoesPredefinidos() {
        JPanel predefPanel = new JPanel(new GridLayout(2, 2, 5, 5));
        predefPanel.setBorder(BorderFactory.createTitledBorder("Configurações Pré-definidas"));
        
        JButton btnNothing = new JButton("NOTHING MODE");
        JButton btnTare = new JButton("TARE MODE");
        JButton btnCalib = new JButton("CALIBRATION MODE");
        JButton btnMotor = new JButton("MOTOR MODE 50%");
        
        btnNothing.addActionListener(e -> setPredefinido(InputData.NOTHING_MODE, 0, 0, 0));
        btnTare.addActionListener(e -> setPredefinido(InputData.TARE_MODE, 0, 0, 0));
        btnCalib.addActionListener(e -> setPredefinido(InputData.CALIBRATION_MODE, 1000, 1000, 0));
        btnMotor.addActionListener(e -> setPredefinido(InputData.MOTOR_MODE, 0, 0, 50));
        
        predefPanel.add(btnNothing);
        predefPanel.add(btnTare);
        predefPanel.add(btnCalib);
        predefPanel.add(btnMotor);
        
        add(predefPanel, BorderLayout.SOUTH);
    }
    
    private void setPredefinido(byte mode, long thrustCal, long torqueCal, int duty) {
        modeComboBox.setSelectedIndex(getIndexFromMode(mode));
        thrustCalField.setText(String.valueOf(thrustCal));
        torqueCalField.setText(String.valueOf(torqueCal));
        dutySlider.setValue(duty);
    }
    
    private int getIndexFromMode(byte mode) {
        switch (mode) {
            case InputData.NOTHING_MODE: return 0;
            case InputData.CALIBRATION_MODE: return 1;
            case InputData.TARE_MODE: return 2;
            case InputData.MOTOR_MODE: return 3;
            default: return 0;
        }
    }
    
    private void enviarConfiguracao(JTextArea statusArea) {
        if (!serial.isConectado()) {
            statusArea.setText("❌ ERRO: Não conectado ao dispositivo!");
            logArea.append("❌ Tentativa de envio sem conexão serial\n");
            return;
        }
        
        try {
            InputData inputData = new InputData();
            
            // Configura modo
            int modoIndex = modeComboBox.getSelectedIndex();
            switch (modoIndex) {
                case 0: inputData.setMode(InputData.NOTHING_MODE); break;
                case 1: inputData.setMode(InputData.CALIBRATION_MODE); break;
                case 2: inputData.setMode(InputData.TARE_MODE); break;
                case 3: inputData.setMode(InputData.MOTOR_MODE); break;
            }
            
            // Configura calibração thrust
            long thrustCal = Long.parseUnsignedLong(thrustCalField.getText().trim());
            inputData.setCalibrationFactorThrust(thrustCal);
            
            // Configura calibração torque
            long torqueCal = Long.parseUnsignedLong(torqueCalField.getText().trim());
            inputData.setCalibrationFactorTorque(torqueCal);
            
            // Configura duty
            inputData.setDuty(dutySlider.getValue());
            
            // Envia dados
            boolean sucesso = serial.enviarDadosConfiguracao(inputData);
            
            if (sucesso) {
                statusArea.setText("✅ Configuração enviada com sucesso!\n" + inputData.toString());
                logArea.append("📤 " + inputData.toString() + "\n");
            } else {
                statusArea.setText("❌ Falha no envio da configuração!");
                logArea.append("❌ Falha no envio da configuração\n");
            }
            
        } catch (NumberFormatException e) {
            statusArea.setText("❌ ERRO: Valores de calibração devem ser números inteiros positivos!");
            logArea.append("❌ Erro de formato nos campos numéricos\n");
        } catch (Exception e) {
            statusArea.setText("❌ ERRO: " + e.getMessage());
            logArea.append("❌ Erro inesperado: " + e.getMessage() + "\n");
        }
    }
    
    private void limparCampos() {
        thrustCalField.setText("0");
        torqueCalField.setText("0");
        dutySlider.setValue(0);
        modeComboBox.setSelectedIndex(0);
    }
}