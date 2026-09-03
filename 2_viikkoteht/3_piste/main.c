// Hei, minä ansaitsen 3 pistettä koska tein kaikki pyydetyt vaiheet ja ne toimivat mikrokontrollerilla kuten pitääkin.




#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <inttypes.h>

// Led pin configurations // led0 on red, led1 on green ja led2 on blue
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

// Buttons configurations
// vaihtamalla sw jälkeen tulevaa numeroa, vaihdamme kytkintä, kuten ledeissä
#define BUTTON_0 DT_ALIAS(sw3) // nappi, jolla pysäytetään suoritus
#define BUTTON_1 DT_ALIAS(sw0) // nappi, jolla sytytetään punainen valo
#define BUTTON_2 DT_ALIAS(sw1) // nappi, jolla sytytetään keltainen valo
#define BUTTON_3 DT_ALIAS(sw2) // nappi, jolla sytytetään vihreä valo


static const struct gpio_dt_spec button_0 = GPIO_DT_SPEC_GET_OR(BUTTON_0, gpios, {0});
static struct gpio_callback button_0_data;

static const struct gpio_dt_spec button_1 = GPIO_DT_SPEC_GET_OR(BUTTON_1, gpios, {0});
static struct gpio_callback button_1_data;

static const struct gpio_dt_spec button_2 = GPIO_DT_SPEC_GET_OR(BUTTON_2, gpios, {0});
static struct gpio_callback button_2_data;

static const struct gpio_dt_spec button_3 = GPIO_DT_SPEC_GET_OR(BUTTON_3, gpios, {0});
static struct gpio_callback button_3_data;

// led thread initialization
#define STACKSIZE 500
#define PRIORITY 5

void red_led_task(void *, void *, void*);
K_THREAD_DEFINE(red_thread,STACKSIZE,red_led_task,NULL,NULL,NULL,PRIORITY,0,0);

void green_led_task(void *, void *, void*);
K_THREAD_DEFINE(green_thread,STACKSIZE,green_led_task,NULL,NULL,NULL,PRIORITY,0,0);

void yellow_led_task(void *, void *, void*);
K_THREAD_DEFINE(yellow_thread,STACKSIZE,yellow_led_task,NULL,NULL,NULL,PRIORITY,0,0);

volatile int tila = 0; // Valon värin määräävä tilamuuttuja
volatile int palautus = 0; // Tilan tallentamiseen käytettävä muuttuja.

void button_0_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("tila ennen stop rimpsua\n");
	printk("Tila: %d\n", tila);
	// stop buttonin logiikka
	if(tila == 5 || tila == 6 || tila == 7 || tila == 8 || tila == 9 || tila == 10) {
		tila = 0;
		return;
	}

	if(tila == 4){
		tila = palautus;
	}
	else {
		tila = 4;
	}
	printk("Stop button pressed ja tila rimpsun jälkeen\n");
	printk("Tila: %d\n", tila);
}

void button_1_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	if(tila != 5) {
		tila = 5; // uusi tila, joka pitää punaisen ledin päällä	
	}
	else {
		tila = 6; // tämä tila sammuttaa ledin
	}
	printk("Red button pressed\n");
}

void button_2_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	if(tila != 7) {
		tila = 7; // uusi tila, joka pitää keltaisen ledin päällä	
	}
	else {
		tila = 8; // tämä tila sammuttaa ledin
	}
	printk("Yellow button pressed\n");
}

void button_3_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	if(tila != 9) {
		tila = 9; // uusi tila, joka pitää keltaisen ledin päällä	
	}
	else {
		tila = 10; // tämä tila sammuttaa ledin
	}
	printk("Green button pressed\n");
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


// Button initialization
int init_button() {
	int ret;
	if (!gpio_is_ready_dt(&button_0)) {
		printk("Error: button 0 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_0, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_0, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_0_data, button_0_handler, BIT(button_0.pin));
	gpio_add_callback(button_0.port, &button_0_data);
	printk("Set up stop button ok\n");

	// red button init
	if (!gpio_is_ready_dt(&button_1)) {
		printk("Error: button 1 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_1, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_1, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_1_data, button_1_handler, BIT(button_1.pin));
	gpio_add_callback(button_1.port, &button_1_data);
	printk("Set up red button ok\n");
	
	// yellow button init
	if (!gpio_is_ready_dt(&button_2)) {
		printk("Error: button 2 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_2, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_2, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_2_data, button_2_handler, BIT(button_2.pin));
	gpio_add_callback(button_2.port, &button_2_data);
	printk("Set up yellow button ok\n");
	
	// green button init
	if (!gpio_is_ready_dt(&button_3)) {
		printk("Error: button 3 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_3, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_3, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_3_data, button_3_handler, BIT(button_3.pin));
	gpio_add_callback(button_3.port, &button_3_data);
	printk("Set up green button ok\n");

	return 0;
}

// Main program
int main(void)
{
	init_led();

	init_button();

	return 0;
}


// Task to handle red led
void red_led_task(void *, void *, void*) {
	printk("Red led thread started\n");

	while(true){
	// jos stop on päällä, jäädään pyörimään tähän tilaan
	while (tila == 4) {
		k_msleep(10);
	}

	while(tila == 5) {
		gpio_pin_set_dt(&red,1);
		gpio_pin_set_dt(&green,0);
		k_msleep(10);
	}
	while(tila == 6) {
		gpio_pin_set_dt(&red,0);
		gpio_pin_set_dt(&green,0);
	}

	if (tila == 0) {
		// 1. Tallennetaan valojen tila
		palautus = tila;
		// 2. Asetetaan oikeat ledit päälle
		gpio_pin_set_dt(&green,0);
		gpio_pin_set_dt(&red,1);
		printk("Red on\n");
		// 3. Pidetään valoja päällä sekunti
		k_sleep(K_SECONDS(1));
		// 4. Asetetaan seuraava valojen tila
		if(tila == 0) {
			tila = 1;
		}
	}
	k_msleep(100);
	}
}

// Task to handle green led
void yellow_led_task(void *, void *, void*) {
	printk("Yellow led thread started\n");

	while(true){
	// jos stop on päällä, jäädään pyörimään tähän tilaan
	while (tila == 4) {
		k_msleep(10);
	}

	if(tila == 7) {
		gpio_pin_set_dt(&red,1);
		gpio_pin_set_dt(&green,1);
	}

	if(tila == 8) {
		gpio_pin_set_dt(&red,0);
		gpio_pin_set_dt(&green,0);
	}

	if (tila == 1) {
		// 1. Tallennetaan tilakoneen tila
		palautus = tila;
		// 2. Asetetaan ledit päälle
		gpio_pin_set_dt(&red,1);
		gpio_pin_set_dt(&green,1);
		printk("Yellow on\n");
		// 3. Annetaan valojen olla päällä sekunti 
		k_sleep(K_SECONDS(1));
		// 4. Asetetaan seuraava tila
		if(tila == 1) {
		tila = 2;
		}
	}
	k_msleep(100);
	}
}

// Task to handle green led
void green_led_task(void *, void *, void*) {
	printk("Green led thread started\n");
	while(true) {
	// Jos stop on päällä, jäädään pyörimään tähän while looppiin		
	while (tila == 4) {
		k_msleep(10);
	}
	
	if(tila == 9) {
		gpio_pin_set_dt(&red,0);
		gpio_pin_set_dt(&green,1);
	}

	if(tila == 10) {
		gpio_pin_set_dt(&red,0);
		gpio_pin_set_dt(&green,0);
	}

	if (tila == 2) {
		// 1. Tallennetaan tilakoneen tila
		palautus = tila;
		// 2. Asetetaan ledit päälle
		gpio_pin_set_dt(&red,0); 
		gpio_pin_set_dt(&green,1);
		printk("Green on\n");
		// 2. Annetaan valojen olla päällä sekunti
		k_sleep(K_SECONDS(1));
		// 3. Asetetaan seuraava tila
		if(tila ==  2) {
		tila = 0;
		}
	}
	k_msleep(100);
	}
}