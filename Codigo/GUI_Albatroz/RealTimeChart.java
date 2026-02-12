import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.ValueAxis;
import org.jfree.chart.plot.XYPlot;
import org.jfree.data.time.Millisecond;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;

import javax.swing.*;
import java.awt.*;

public class RealTimeChart extends JPanel {
    private TimeSeries series;
    private JFreeChart chart;
    private String title;
    private String yAxisLabel;

    public RealTimeChart(String title, String yAxisLabel) {
        this.title = title;
        this.yAxisLabel = yAxisLabel;
        criarChart();
    }

    private void criarChart() {
        // Cria a série temporal
        series = new TimeSeries("Dados");

        // Cria o dataset
        TimeSeriesCollection dataset = new TimeSeriesCollection(series);

        // Cria o chart
        chart = ChartFactory.createTimeSeriesChart(
            title,
            "Tempo",
            yAxisLabel,
            dataset,
            true,
            true,
            false
        );

        // Customiza o chart
        chart.setBackgroundPaint(Color.WHITE);
        
        XYPlot plot = chart.getXYPlot();
        plot.setBackgroundPaint(Color.LIGHT_GRAY);
        plot.setDomainGridlinePaint(Color.WHITE);
        plot.setRangeGridlinePaint(Color.WHITE);
        
        // Configura o eixo Y
        ValueAxis rangeAxis = plot.getRangeAxis();
        rangeAxis.setAutoRange(true);

        // Cria o panel do chart
        ChartPanel chartPanel = new ChartPanel(chart);
        chartPanel.setPreferredSize(new Dimension(600, 300));
        
        setLayout(new BorderLayout());
        add(chartPanel, BorderLayout.CENTER);
    }

    public void adicionarPonto(double valor) {
        series.addOrUpdate(new Millisecond(), valor);
        
        // Mantém apenas os últimos 100 pontos no gráfico
        if (series.getItemCount() > 100) {
            series.delete(0, 0);
        }
    }

    public void limpar() {
        series.clear();
    }
}