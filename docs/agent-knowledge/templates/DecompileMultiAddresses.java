import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import java.io.File;
import java.io.FileWriter;

public class DecompileMultiAddresses extends GhidraScript {
    public void run() throws Exception {
        String[] addrs = getScriptArgs()[0].split(",");
        String outDir = getScriptArgs()[1];
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        for (String a : addrs) {
            Address addr = currentProgram.parseAddress(a.trim());
            Function fn = getFunctionAt(addr);
            DecompileResults r = di.decompileFunction(fn, 300, null);
            String c = r.getDecompiledFunction().getC();
            FileWriter fw = new FileWriter(new File(outDir, "decomp_" + a.trim() + ".c"));
            fw.write(c);
            fw.close();
        }
        di.dispose();
    }
}
