import ghidra.app.script.GhidraScript;
import ghidra.app.services.ConsoleService;

public class analyze_hamsterball extends GhidraScript {
    public void run() throws Exception {
        ConsoleService console = getConsole();
        println("Hamsterball.exe Analysis Started");
        println("Entry point: " + currentProgram.getEntryPoint());
        println("Image base: " + Long.toHexString(currentProgram.getImageBase().offset));
        
        // List function counts
        int funcCount = getFunctionManager().getFunctionCount();
        println("Total functions: " + funcCount);
        
        // Get first 20 functions
        println("\nFirst 20 functions:");
        FunctionIterator iter = getFunctionManager().getFunctions(true);
        int count = 0;
        while (iter.hasNext() && count < 20) {
            Function f = iter.next();
            println("  " + f.getEntryPoint() + ": " + f.getName());
            count++;
        }
    }
}
