// package Week3;

public class BankImpl {
	private int numberOfCustomers;	// the number of customers
	private int numberOfResources;	// the number of resources

	private int[] available; 	// the available amount of each resource
	private int[][] maximum; 	// the maximum demand of each customer
	private int[][] allocation;	// the amount currently allocated
	private int[][] need;		// the remaining needs of each customer
	
	public BankImpl (int[] resources, int numberOfCustomers) {
		// set the number of resources
		this.numberOfResources = resources.length;

		// set the number of customers
		this.numberOfCustomers = numberOfCustomers;

		// set the value of bank resources to available
		this.available = new int[numberOfResources];
		for(int i = 0; i < this.numberOfResources; i++){
			this.available[i] = resources[i];
		}

		// set the array size for maximum, allocation, and need
		this.maximum = new int[numberOfCustomers][numberOfResources];
		this.allocation = new int[numberOfCustomers][numberOfResources];
		this.need = new int[numberOfCustomers][numberOfResources];
	}
	
	public int getNumberOfCustomers() {
		//return numberOfCustomers
		return this.numberOfCustomers;
	}

	public void addCustomer(int customerNumber, int[] maximumDemand) {
		// add customer, update maximum and need
		for(int i = 0; i < this.numberOfResources; i++){
			this.maximum[customerNumber][i] = maximumDemand[i];
			this.need[customerNumber][i] = maximumDemand[i];
		}
	}

	public void getState() {
		// print the current state with a tidy format
		System.out.print("\nCurrent state: \n");
		// print available
		System.out.print("Available: \n");
		for (int i = 0; i < numberOfResources; i++) {
			System.out.print(available[i] + " ");
		}
		System.out.print("\n");
		// print maximum
		System.out.print("\nmaximum: \n");
		for (int i = 0; i < numberOfCustomers; i++) {
			for(int j = 0; j < numberOfResources; j++) {
				System.out.print(maximum[i][j] + " ");
			}
			System.out.print("\n");		
		}
		// print allocation
		System.out.print("\nAllocation: \n");
		for (int i = 0; i < numberOfCustomers; i++) {
			for(int j = 0; j < numberOfResources; j++) {
				System.out.print(allocation[i][j] + " ");
			}
			System.out.print("\n");		
		}
		// print need
		System.out.print("\nNeed: \n");
		for (int i = 0; i < numberOfCustomers; i++) {
			for(int j = 0; j < numberOfResources; j++) {
				System.out.print(need[i][j] + " ");
			}
			System.out.print("\n");		
		}	
		System.out.print("\n");
	}

	public synchronized boolean requestResources(int customerNumber, int[] request) {
		// print the request
		System.out.print("\nRequest resource, customerNumber " + customerNumber + ": [" );
		for(int i = 0; i < this.numberOfResources-1; i++) {
			System.out.print(request[i] + " ");
		}
		System.out.print(request[this.numberOfResources-1] + "]\n");
		// check if request larger than need
		for(int i = 0; i < this.numberOfResources; i++){
			if(request[i] > this.need[customerNumber][i]){
				System.out.print("Request from customer "+customerNumber+" exceeds the need for resource "+i+"!\n");
				return false;
			}
		}
		// check if request larger than available
		for(int i = 0; i < this.numberOfResources; i++){
			if(request[i] > this.available[i]){
				System.out.print("Request from customer "+customerNumber+" is more than avaliable for resource "+i+"!\n");
				return false;
			}
		}
		// TODO: check if the state is safe or not
		if (!this.checkSafe(customerNumber, request)){
			System.out.print("Request may cause the system unsafe. Rejected.\n");
			return false;
		}
		// if it is safe, allocate the resources to customer customerNumber 
		System.out.print("Request granted.\n");
		for(int i = 0; i < this.numberOfResources; i++){
			this.available[i] -= request[i];
			this.allocation[customerNumber][i] += request[i];
			this.need[customerNumber][i] -= request[i];
		}
		return true;
	}

	public synchronized void releaseResources(int customerNumber, int[] release) {
		// print the release
		// release the resources from customer customerNumber 
		System.out.print("\nRelease resource, customerNumber %"+customerNumber+": [");
		for(int i = 0; i < this.numberOfResources-1; i++) {
			System.out.print("release[i], ");
		}
		System.out.print(release[this.numberOfResources-1] + "]\n");

		for (int i = 0; i < this.numberOfResources; i++){
			this.available[i] += release[i];
			this.allocation[customerNumber][i] -= release[i];
			this.need[customerNumber][i] += release[i];
		}
	}

	private synchronized boolean checkSafe(int customerNumber, int[] request) {
		// TODO: check if the state is safe
		int[] temp_avail = new int[10];
		int[][] temp_need = new int[10][10];
		int[][] temp_allocation = new int[10][10];
		int[] work = new int[10];
		boolean[] finish = new boolean[10];
		boolean possible = true;

		for(int j = 0; j < this.numberOfResources; j++){
			temp_avail[j] = this.available[j] - request[j];
			work[j] = temp_avail[j];
			for(int i = 0; i < numberOfCustomers; i++){
				if (i == customerNumber){
					temp_need[customerNumber][j] = this.need[customerNumber][j] - request[j];
					temp_allocation[customerNumber][j] = this.allocation[customerNumber][j] + request[j];
				}
				else{
					temp_need[i][j] = this.need[i][j];
					temp_allocation[i][j] = this.allocation[i][j];
				}
			}
		}

		for (int i = 0; i < this.numberOfCustomers; i++){
			finish[i] = false;
		}

		while(possible){
			possible = false;
			for (int i = 0; i < this.numberOfCustomers; i++){
				boolean feasible = true;
				for(int j = 0; j < this.numberOfResources; j++){
					if(temp_need[i][j] > work[j]){
						feasible = false;
					}
				}
				if(!finish[i] && feasible){
					possible = true;
					for(int j = 0; j < this.numberOfResources; j++){
						work[j] += temp_allocation[i][j];
					}
					finish[i] = true;
				}
			}
		}
		boolean safe = true;
		for (int i = 0; i < this.numberOfCustomers; i++){
			if(!finish[i]){
				safe = false;
			}
		}
		return safe;
	}
}