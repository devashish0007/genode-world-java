import java.util.Scanner;
import java.util.concurrent.TimeUnit;


class app_java
{
	public static void main(String args[])
	{
		System.err.println("Java APP started");

		Scanner scan = new Scanner(System.in);

		char   in   = '\0';
		String data = "";
		while (in != ';') {

			try {
				in = scan.next().charAt(0);
				data += in;

				System.err.println(data);

				break;
			} catch (java.util.NoSuchElementException e) {
			}

			try {
				TimeUnit.SECONDS.sleep(1);
			} catch (InterruptedException e) {
			}
		}

		String result = data + "--42--;";
		System.out.println(result);

		scan.close();

		System.err.println("Java APP finished");
	}
}
