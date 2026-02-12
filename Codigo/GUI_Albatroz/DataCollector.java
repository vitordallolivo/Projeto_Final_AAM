import java.util.LinkedList;
import java.util.Queue;

public class DataCollector {
    private Queue<Double> dataQueue;
    private int maxDataPoints;
    private double lastValue;

    public DataCollector(int maxPoints) {
        this.maxDataPoints = maxPoints;
        this.dataQueue = new LinkedList<>();
        this.lastValue = 0.0;
    }

    public void addData(double value) {
        dataQueue.offer(value);
        lastValue = value;
        
        // Mantém apenas os últimos 'maxDataPoints' valores
        while (dataQueue.size() > maxDataPoints) {
            dataQueue.poll();
        }
    }

    public double[] getData() {
        double[] data = new double[dataQueue.size()];
        int i = 0;
        for (Double value : dataQueue) {
            data[i++] = value;
        }
        return data;
    }

    public double getLastValue() {
        return lastValue;
    }

    public void clear() {
        dataQueue.clear();
        lastValue = 0.0;
    }
}