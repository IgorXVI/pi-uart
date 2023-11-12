#include <linux/module.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/serdev.h>
#include <linux/proc_fs.h>
#include <linux/minmax.h>
#include <linux/mutex.h>

// informações sobre o módulo de kernel
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Igor Tomelero de Almeida");
MODULE_DESCRIPTION("Simples modulo que recebe chars por UART e salva eles em um buffer.");

#define READ_BUFFER_MAX_SIZE 255
#define WRITE_BUFFER_MAX_SIZE 255UL
#define PROC_FILE_NAME "pi-uart-data"
#define BAUDRATE 9600
#define DELETE_MESSAGE "All previous messages erased.\0"
#define MAX_SIZE_MESSAGE "Max buffer size reached!\0"

static struct mutex global_mutex;

static char proc_read_buffer[READ_BUFFER_MAX_SIZE];
static int proc_read_buffer_size = 0;

static char proc_write_buffer[WRITE_BUFFER_MAX_SIZE];
static int proc_write_buffer_size = 0;

// quando o usuário tentar ler do arquivo proc, exemplo: "cat /proc/<PROC_FILE_NAME>",
// essa função vai ser chamada, ela simplismente retorna todo
// o buffer na primeira vez que for chamada e depois retorna o valor 0
static ssize_t proc_read(struct file *file_pointer, char *user_buffer, size_t count, loff_t *offset)
{
	printk("pi_uart - proc file - read was called!\n");

	mutex_lock(&global_mutex);

	if (*offset >= proc_read_buffer_size || copy_to_user(user_buffer, proc_read_buffer, proc_read_buffer_size))
	{
		mutex_unlock(&global_mutex);

		printk("pi_uart - proc file - copy_to_user ended\n");

		return 0;
	}
	else
	{
		*offset += proc_read_buffer_size;
	}

	mutex_unlock(&global_mutex);

	return proc_read_buffer_size;
}

static ssize_t proc_write(struct file *file_pointer, const char *user_buffer, size_t count, loff_t *offset)
{
	printk("pi_uart - proc file - write was called!\n");

	mutex_lock(&global_mutex);

	proc_write_buffer_size = min(WRITE_BUFFER_MAX_SIZE, count);

	if (copy_from_user(proc_write_buffer, user_buffer, proc_write_buffer_size))
	{
		mutex_unlock(&global_mutex);

		return -EFAULT;
	}

	*offset += proc_write_buffer_size;

	if (proc_write_buffer_size < WRITE_BUFFER_MAX_SIZE)
	{
		proc_write_buffer[proc_write_buffer_size] = '\0';
		proc_write_buffer_size++;
	}
	else
	{
		proc_write_buffer[WRITE_BUFFER_MAX_SIZE - 1] = '\0';
	}

	mutex_unlock(&global_mutex);

	return proc_write_buffer_size;
}

// função que recebe os bytes por UART e salva o primeiro byte recebido no buffer
static int receive_buf(struct serdev_device *serdev, const unsigned char *received_buffer, size_t number_of_bytes_received)
{
	char received_char = (char)(*received_buffer);

	printk("pi_uart - Received char %c\n", received_char);

	mutex_lock(&global_mutex);

	if (received_char == '~')
	{
		proc_read_buffer_size = 0;

		serdev_device_write_buf(serdev, DELETE_MESSAGE, sizeof(DELETE_MESSAGE));

		goto RECEIVE_END;
	}

	if (received_char == '^')
	{
		serdev_device_write_buf(serdev, proc_read_buffer, proc_read_buffer_size + 1);

		goto RECEIVE_END;
	}

	if (received_char == '`')
	{
		serdev_device_write_buf(serdev, proc_write_buffer, proc_write_buffer_size);

		goto RECEIVE_END;
	}

	if (proc_read_buffer_size >= READ_BUFFER_MAX_SIZE - 1)
	{
		serdev_device_write_buf(serdev, MAX_SIZE_MESSAGE, sizeof(MAX_SIZE_MESSAGE));

		goto RECEIVE_END;
	}

	proc_read_buffer[proc_read_buffer_size] = received_char;

	proc_read_buffer[proc_read_buffer_size + 1] = '\0';

	proc_read_buffer_size++;

RECEIVE_END:
	mutex_unlock(&global_mutex);

	return number_of_bytes_received;
}

static struct proc_ops pi_uart_proc_fops = {
	.proc_read = proc_read,
	.proc_write = proc_write,
};

static const struct serdev_device_ops pi_uart_ops = {
	.receive_buf = receive_buf,
};

//----------------------------------------------------------------------------INICIALIZAÇÂO---------------------------------------------------------------------------------------------------------------------------
// O código abaixo vai lidar apenas com a inicialização do arquivo proc e do serdev

// Necessário declarar as funções do serdev antes de escreve-las

static int pi_uart_probe(struct serdev_device *serdev);
static void pi_uart_remove(struct serdev_device *serdev);

// atributo "compatible" deve ter o mesmo valor do que está no arquivo de overlay (dts)
static struct of_device_id pi_uart_ids[] = {
	{.compatible = "brightlight,echodev"},
	// null entry no fim do array
	{},
};

MODULE_DEVICE_TABLE(of, pi_uart_ids);

// declaração das operações do serdev, driver.name pode ser qualquer coisa, mas é melhor que seja algo relacionado ao modulo
static struct serdev_device_driver pi_uart_driver = {
	.probe = pi_uart_probe,
	.remove = pi_uart_remove,
	.driver = {
		.name = "pi-uart",
		.of_match_table = pi_uart_ids,
	},
};

// Essa função vai ser chamada quando o serdev for registrado (na função my_init)
static int pi_uart_probe(struct serdev_device *serdev)
{
	printk("pi_uart - now im in the probe function!\n");

	// registra a operação de receber dados
	serdev_device_set_client_ops(serdev, &pi_uart_ops);

	// tenta inicializar o serdev
	if (serdev_device_open(serdev) > 1)
	{
		printk("pi_uart - error when opening the serial device!\n");

		return -1;
	}

	// determina o baud rate
	// um baud é uma medida de velocidade de sinalização e representa o número de mudanças na linha de transmissão ou eventos por segundo
	serdev_device_set_baudrate(serdev, BAUDRATE);

	// determina que não tem flow control
	serdev_device_set_flow_control(serdev, false);

	// determina que não tem bit de paridade
	serdev_device_set_parity(serdev, SERDEV_PARITY_NONE);

	return 0;
}

// Essa função vai ser chamada quando o serdev for removido (na função my_exit)
static void pi_uart_remove(struct serdev_device *serdev)
{
	printk("pi_uart - now im in the remove function!\n");

	serdev_device_close(serdev);
}

// arquivo proc que vai ser usado para ler o buffer
static struct proc_dir_entry *proc_file;

static int __init my_init(void)
{
	printk("pi_uart - Hello, Kernel!\n");

	// cria a proc file no path /proc/<PROC_FILE_NAME>
	proc_file = proc_create(PROC_FILE_NAME, 0666, NULL, &pi_uart_proc_fops);

	if (proc_file == NULL)
	{
		printk("pi_uart - Error creating proc file!\n");

		return -1;
	}

	mutex_init(&global_mutex);

	if (serdev_device_driver_register(&pi_uart_driver))
	{
		printk("pi_uart - could not load driver!\n");

		mutex_destroy(&global_mutex);

		proc_remove(proc_file);

		return -1;
	}

	return 0;
}

static void __exit my_exit(void)
{
	printk("pi_uart - Goodbye, Kernel\n");

	mutex_destroy(&global_mutex);

	proc_remove(proc_file);

	serdev_device_driver_unregister(&pi_uart_driver);
}

module_init(my_init);
module_exit(my_exit);