#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>

// Led pin configurations // led0 on red, led1 on green ja led2 on blue
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

// Condition Variables
K_MUTEX_DEFINE(red_mutex);
K_CONDVAR_DEFINE(red_signal);

K_MUTEX_DEFINE(green_mutex);
K_CONDVAR_DEFINE(green_signal);

K_MUTEX_DEFINE(yellow_mutex);
K_CONDVAR_DEFINE(yellow_signal);

// semafori
K_SEM_DEFINE(release_sem, 0, 1);

/****************************
 * Remember to add line:
 * CONFIG_HEAP_MEM_POOL_SIZE=1024
 * to prj.conf
 ****************************/

// Thread initializations
#define STACKSIZE 500
#define PRIORITY 5

// LED task init
void red_task(void *, void *, void*);
K_THREAD_DEFINE(red_thread,STACKSIZE,red_task,NULL,NULL,NULL,PRIORITY,0,0);
void green_task(void *, void *, void*);
K_THREAD_DEFINE(green_thread,STACKSIZE,green_task,NULL,NULL,NULL,PRIORITY,0,0);
void yellow_task(void *, void *, void*);
K_THREAD_DEFINE(yellow_thread,STACKSIZE,yellow_task,NULL,NULL,NULL,PRIORITY,0,0);

// UART initialization
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

// Create dispatcher FIFO buffer
K_FIFO_DEFINE(dispatcher_fifo);

// FIFO dispatcher data type
struct data_t {
	/*************************
	// Add fifo_reserved below
	*************************/
	void *fifo_reserved;
	char msg[20];
};

/********************
 * init UART
 */
int init_uart(void) {
	// UART initialization
	if (!device_is_ready(uart_dev)) {
		return 1;
	} 
	return 0;
}

// Initialize leds
int  init_led() {
	// Led pin initialization
	int ret = gpio_pin_configure_dt(&red, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error: Led configure failed\n");		
		return ret;
	}
	// set led off
	gpio_pin_set_dt(&red,0);

	// Led pin initialization
	ret = gpio_pin_configure_dt(&green, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error: Led configure failed\n");		
		return ret;
	}
	// set led off
	gpio_pin_set_dt(&green,0);

	printk("Led initialized ok\n");
	
	return 0;
}

/********************
 * Main task
 */
int main(void)
{
	int ret = init_uart();
	if (ret != 0) {
		printk("UART initialization failed!\n");
		return ret;
	}
	ret = init_led();
	if (ret != 0) {
		printk("LED inilialization failed!\n");
		return ret;
	}

	return 0;
}

/********************
 * UART task
 */
static void uart_task(void *unused1, void *unused2, void *unused3)
{
	// Received character from UART
	char rc=0;
	// Message from UART
	char uart_msg[20];
	memset(uart_msg,0,20);
	int uart_msg_cnt = 0;

	while (true) {
		// Ask UART if data available
		if (uart_poll_in(uart_dev,&rc) == 0) {
			// printk("Received: %c\n",rc);
			// If character is not newline, add to UART message buffer
			if (rc != '\r') {
				uart_msg[uart_msg_cnt] = rc;
				uart_msg_cnt++;
			// Character is newline, copy dispatcher data and put to FIFO buffer
			} else {
				printk("UART msg: %s\n", uart_msg);
                
				struct data_t *buf = k_malloc(sizeof(struct data_t));
				if (buf == NULL) {
					return;
				}
				// Copy UART message to dispatcher data
				// strncpy(buf->msg, 20, uart_msg); // mitä ihmettä, miksi kaatuu!!
				snprintf(buf->msg, 20, "%s", uart_msg);

				// You need to:
				// Put dispatcher data to FIFO buffer
				k_fifo_put(&dispatcher_fifo, buf);
				// Clear UART receive buffer
				uart_msg_cnt = 0;
				memset(uart_msg,0,20);

				// Clear UART message buffer
				uart_msg_cnt = 0;
				memset(uart_msg,0,20);
			}
		}
		k_msleep(10);
	}
	return 0;
}

/********************
 * Dispatcher task
 */
static void dispatcher_task(void *unused1, void *unused2, void *unused3)
{
	while (true) {
		// Receive dispatcher data from uart_task fifo
		struct data_t *rec_item = k_fifo_get(&dispatcher_fifo, K_FOREVER);
		char sequence[20];
		memcpy(sequence,rec_item->msg,20);
		k_free(rec_item);

		printk("Dispatcher: %s\n", sequence);
        
		int cnt = 0;

		// tulostetaan merkki kerrallaan
		while(sequence[cnt] != 0) {
			if(sequence[cnt] == 'R')
			{
				printk("RED");
				k_condvar_broadcast(&red_signal);
				k_sem_take(&release_sem, K_FOREVER);
			}
			if(sequence[cnt] == 'G')
			{
				printk("GREEN");
				k_condvar_broadcast(&green_signal);
				k_sem_take(&release_sem, K_FOREVER);
			}
			if(sequence[cnt] == 'Y')
			{
				printk("YELLOW");
				k_condvar_broadcast(&yellow_signal);
				k_sem_take(&release_sem, K_FOREVER);
			}
			//printk("%c\n", sequence[cnt]);
			cnt++;
		}

		// You need to:
        // Parse color and time from the fifo data
        // Example
        //    char color = sequence[0];
        //    int time = atoi(sequence+2);
		//    printk("Data: %c %d\n", color, time);
        // Send the parsed color information to tasks using fifo
        // Use release signal to control sequence or k_yield
	}
}

K_THREAD_DEFINE(dis_thread,STACKSIZE,dispatcher_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(uart_thread,STACKSIZE,uart_task,NULL,NULL,NULL,PRIORITY,0,0);


// led tasks
void green_task(void *, void *, void*) {
	while (true) {
		// Wait for signal
		k_condvar_wait(&green_signal, &green_mutex, K_FOREVER);
		printk("Green thread runs\n");
		// 2. Asetetaan ledit päälle 
		gpio_pin_set_dt(&green,1);
		printk("Green on\n");
		// 3. Annetaan valojen olla päällä sekunti
		k_sleep(K_SECONDS(1));
		gpio_pin_set_dt(&green, 0);
		k_sem_give(&release_sem); 
		// Sleep nor yield not needed
		// k_yield();
	}
}

void yellow_task(void *, void *, void*) {
	while(true){
		k_condvar_wait(&yellow_signal, &yellow_mutex, K_FOREVER);
		// 2. Asetetaan ledit päälle
		gpio_pin_set_dt(&red,1);
		gpio_pin_set_dt(&green,1);
		printk("Yellow thread runs\n");
		// 3. Annetaan valojen olla päällä sekunti 
		k_sleep(K_SECONDS(1));
		gpio_pin_set_dt(&red,0);
		gpio_pin_set_dt(&green,0);
		k_sem_give(&release_sem); 
	}
}


void red_task(void *, void *, void*) {
	while (true) {
		// Wait for signal
		k_condvar_wait(&red_signal, &red_mutex, K_FOREVER);
		printk("Red thread runs\n");
 		// 2. Asetetaan ledit päälle 
		gpio_pin_set_dt(&red,1);
		printk("Red on\n");
		// 3. Annetaan valojen olla päällä sekunti
		k_sleep(K_SECONDS(1));
		gpio_pin_set_dt(&red, 0);
		// semafori
		k_sem_give(&release_sem); 
		// Sleep nor yield not needed
		// k_yield();
	}
}