// package Week3;

public class BankImpl {
	private int numberOfCustomers;	// the number of customers
	private int numberOfResources;	// the number of resources

	private int[] available; 	// the available amount of each resource
	private int[][] maximum; 	// the maximum demand of each customer
	private int[][] allocation;	// the amount currently allocated
	private int[][] need;		// the remaining needs of each customer
	
	public BankImpl (int[] resources, int numberOfCustomers) {
		// TODO: set the number of resources

		// TODO: set the number of customers

		// TODO: set the value of bank resources to available

		// TODO: set the array size for maximum, allocation, and need

	}
	
	public int getNumberOfCustomers() {
		// TODO: return numberOfCustomers
	}

	public void addCustomer(int customerNumber, int[] maximumDemand) {
		// TODO: add customer, update maximum and need

	}

	public void getState() {
		// TODO: print the current state with a tidy format

		// TODO: print available

		// TODO: print maximum

		// TODO: print allocation

		// TODO: print need

	}

	public synchronized boolean requestResources(int customerNumber, int[] request) {
		// TODO: print the request

		// TODO: check if request larger than need

		// TODO: check if request larger than available

		// TODO: check if the state is safe or not

		// TODO: if it is safe, allocate the resources to customer customerNumber 

	}

	public synchronized void releaseResources(int customerNumber, int[] release) {
		// TODO: print the release
		// release the resources from customer customerNumber 

	}

	private synchronized boolean checkSafe(int customerNumber, int[] request) {
		// TODO: check if the state is safe

	}
}