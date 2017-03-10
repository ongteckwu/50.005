import java.io.FileNotFoundException;
import java.util.ArrayList;

public class MeanThread {	
	public static void main(String[] args) throws InterruptedException, FileNotFoundException {
		// TODO: read data from external file and store it in an array
		       // Note: you should pass the file as a first command line argument at runtime. 
		
		// define number of threads
		int NumOfThread = Integer.valueOf(args[2]);// this way, you can pass number of threads as 
		     // a second command line argument at runtime.
		
		// TODO: partition the array list into N subArrays, where N is the number of threads
		
		// TODO: start recording time
		
		// TODO: create N threads and assign subArrays to the threads so that each thread computes mean of 
		    // its repective subarray. For example,
		
		MeanMultiThread thread1 = new MeanMultiThread(subArray1);
		MeanMultiThread threadn = new MeanMultiThread(subArrayn);
		//Tip: you can't create big number of threads in the above way. So, create an array list of threads. 
		
		// TODO: start each thread to execute your computeMean() function defined under the run() method
		   //so that the N mean values can be computed. for example, 
		thread1.start(); //start thread1 on from run() function
		threadn.start();//start thread2 on from run() function
		
		thread1.join();//wait until thread1 terminates
		threadn.join();//wait until threadn terminates
		
		// TODO: show the N mean values
		System.out.println("Temporal mean value of thread n is ... ");	
		
		// TODO: store the temporal mean values in a new array so that you can use that 
		    /// array to compute the global mean.
		
		// TODO: compute the global mean value from N mean values. 
		
		// TODO: stop recording time and compute the elapsed time 
		
		System.out.println("The global mean value is ... ");
				
	}
}
//Extend the Thread class
class MeanMultiThread extends Thread {
	private ArrayList<Integer> list;
	private double mean;
	MeanMultiThread(ArrayList<Integer> array) {
		list = array;
	}
	public double getMean() {
		return mean;
	}
	public void run() {
		// TODO: implement your actions here, e.g., computeMean(...)
		mean = computeMean(list);
	}
}