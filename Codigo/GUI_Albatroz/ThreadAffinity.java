import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.Future;
import java.util.concurrent.atomic.AtomicBoolean;

public class ThreadAffinity {
    
    public static boolean setThreadAffinity(int coreId) {
        try {
            if (coreId < 0) return false;
            
            // Para Windows
            if (System.getProperty("os.name").toLowerCase().contains("win")) {
                return setWindowsAffinity(coreId);
            }
            // Para Linux
            else if (System.getProperty("os.name").toLowerCase().contains("linux")) {
                return setLinuxAffinity(coreId);
            }
            // Para outras plataformas
            else {
                return setGenericAffinity(coreId);
            }
        } catch (Exception e) {
            System.err.println("Erro ao definir afinidade: " + e.getMessage());
            return false;
        }
    }
    
    private static boolean setWindowsAffinity(int coreId) {
        try {
            Process process = Runtime.getRuntime().exec(
                new String[]{"cmd", "/c", "wmic process where processid=" + 
                 ProcessHandle.current().pid() + " CALL setpriority " + coreId}
            );
            return process.waitFor() == 0;
        } catch (Exception e) {
            return false;
        }
    }
    
    private static boolean setLinuxAffinity(int coreId) {
        try {
            Process process = Runtime.getRuntime().exec(
                new String[]{"taskset", "-cp", String.valueOf(coreId), 
                 String.valueOf(ProcessHandle.current().pid())}
            );
            return process.waitFor() == 0;
        } catch (Exception e) {
            return false;
        }
    }
    
    private static boolean setGenericAffinity(int coreId) {
        int availableCores = Runtime.getRuntime().availableProcessors();
        if (coreId >= availableCores) {
            return false;
        }
        
        Thread currentThread = Thread.currentThread();
        System.out.println("Thread '" + currentThread.getName() + 
                          "' deveria executar no core: " + coreId + 
                          " (Total de cores: " + availableCores + ")");
        return true;
    }
    
    public static int getAvailableCores() {
        return Runtime.getRuntime().availableProcessors();
    }
}