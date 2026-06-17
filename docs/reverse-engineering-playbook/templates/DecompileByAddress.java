import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import java.io.File;
import java.io.FileWriter;

public class DecompileByAddress extends GhidraScript {
    public void run() throws Exception {
        String addrStr = getScriptArgs()[0];
        String outPath = getScriptArgs()[1];
        Address addr = currentProgram.parseAddress(addrStr);
        Function fn = getFunctionAt(addr);
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        DecompileResults r = di.decompileFunction(fn, 300, null);
        String c = r.getDecompiledFunction().getC();
        FileWriter fw = new FileWriter(new File(outPath));
        fw.write(c);
        fw.close();
        di.dispose();
    }
}
