#include <linux/module.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/serdev.h>
#include <linux/proc_fs.h>

//informações sobre o módulo de kernel
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Igor Tomelero de Almeida");
MODULE_DESCRIPTION("Simples modulo que recebe chars por UART e salva eles num ring buffer, quando o arquivo proc é lido os dados do ring buffer são retornados.");

//inicialização do ring buffer
static char global_buffer[255];
static int global_buffer_head = 0;
static int global_buffer_size = 0;

//arquivo proc que vai ser usado para ler o buffer
static struct proc_dir_entry *proc_file;

//quando o usuário tentar ler do arquivo proc, exemplo: "cat /proc/pi-uart-data",
//essa função vai ser chamada, ela simplismente retorna todo 
//o buffer na primeira vez que for chamada e depois retorna o valor 0
static ssize_t proc_read(struct file *file_pointer, char *user_buffer, size_t count, loff_t *offset) {
	printk("pi_uart - file /proc/pi-uart-data - read was called!\n");

	if (*offset >= global_buffer_size || copy_to_user(user_buffer, global_buffer, global_buffer_size)) {
		printk("pi_uart - file /proc/pi-uart-data - copy_to_user ended\n");
		return 0;
	} else {
		*offset += global_buffer_size;
	}

	return global_buffer_size;
}

//declaração das operações no arquivo proc
static struct proc_ops pi_uart_proc_fops = {
	.proc_read = proc_read
};

//Necessário declarar as funções do serdev antes de escreve-las

static int pi_uart_probe(struct serdev_device *serdev);
static void pi_uart_remove(struct serdev_device *serdev);

//atributo "compatible" deve ter o mesmo valor do que está no arquivo de overlay
static struct of_device_id pi_uart_ids[] = {
	{
		.compatible = "brightlight,echodev"
	},
	//null entry no fim do array
	{}
};

MODULE_DEVICE_TABLE(of, pi_uart_ids);

//declaração das operações do serdev, driver.name pode ser qualquer coisa, mas é melhor que seja algo relacionado ao modulo
static struct serdev_device_driver pi_uart_driver = {
	.probe = pi_uart_probe,
	.remove = pi_uart_remove,
	.driver = {
		.name = "pi-uart",
		.of_match_table = pi_uart_ids
	}
};

//função que recebe os bytes por UART e salva o último byte recebido no buffer
//quando o buffer estiver cheio a função vai voltar o inicio e começar a
//sobreescrever os dados que estavam no inicio
static int receive_data_by_uart_and_save_last_byte(struct serdev_device *serdev, const unsigned char *buffer, size_t size) {
	char *last_char_ptr = buffer + size - 1;
	char last_char = (char)(*last_char_ptr);

	char message[2] = {last_char, '\0'};

	printk("pi_uart - Received %ld bytes with \"%s\"\n", size, message);

	//volta o ponteiro de escrita para o início do buffer global
	//se o tamanho máximo ja tiver sido atingido
	if (global_buffer_head >= 255) {
		global_buffer_head = 0;
	}

	//escreve onde o ponteiro de escrita está apontando no buffer global
	global_buffer[global_buffer_head] = last_char;

	//só incrementa o valor do tamanho salvo no buffer global se o ponteiro
	//não tiver voltado para o começo ainda
	if (global_buffer_size == global_buffer_head) {
		global_buffer_size++;
	}

	//incrementa o ponteiro de escrita para a próxima vez que essa função for chamada
	global_buffer_head++;

	return size;
}

static const struct serdev_device_ops pi_uart_ops = {
	.receive_buf = receive_data_by_uart_and_save_last_byte
};

//Essa função vai ser chamada quando o serdev for registrado (na função my_init)
static int pi_uart_probe(struct serdev_device *serdev) {
	int status;

	printk("pi_uart - now im in the probe function!\n");

	//cria a proc file no path /proc/pi-uart-data
	proc_file = proc_create("pi-uart-data", 0666, NULL, &pi_uart_proc_fops);
	if(proc_file == NULL) {
		printk("pi_uart - Error creating /proc/pi-uart-data\n");
		return -ENOMEM;
	}

	//registra a operação de receber dados
	serdev_device_set_client_ops(serdev, &pi_uart_ops);

	//tenta inicializar o serdev
	status = serdev_device_open(serdev);

	if(status > 1) {
		printk("pi_uart - error when opening the serial device!\n");
		return -1;
	}

	//determina o baud rate como 9600
	//um baud é uma medida de velocidade de sinalização e representa o número de mudanças na linha de transmissão ou eventos por segundo
	serdev_device_set_baudrate(serdev, 9600);

	//determina que não tem flow control
	serdev_device_set_flow_control(serdev, false);

	//determina que não tem bit de paridade
	serdev_device_set_parity(serdev, SERDEV_PARITY_NONE);

	return 0;
}

//Essa função vai ser chamada quando o serdev for removido (na função my_exit)
static void pi_uart_remove(struct serdev_device *serdev) {
	printk("pi_uart - now im in the remove function!\n");

	//apaga o arquivo /proc/pi-uart-data
	proc_remove(proc_file);

	//mata o serdev
	serdev_device_close(serdev);
}

static int __init my_init(void) {
	printk("pi_uart - Hello, Kernel!\n");

	if(serdev_device_driver_register(&pi_uart_driver)) {
		printk("pi_uart - could not load driver!\n");
		return -1;
	}

	return 0;
}

static void __exit my_exit(void) {
	printk("pi_uart - Goodbye, Kernel\n");

	serdev_device_driver_unregister(&pi_uart_driver);
}

module_init(my_init);
module_exit(my_exit);
