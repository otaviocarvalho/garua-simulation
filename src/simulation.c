#include "simgrid/msg.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(msg_simulation, "Messages specific for this msg example");

static int cloud(int argc, char *argv[])
{
  long number_of_tasks = xbt_str_parse_int(argv[1], "Invalid amount of tasks: %s");    /** - Number of tasks      */
  double comp_size = xbt_str_parse_double(argv[2], "Invalid computational size: %s");  /** - Task compute cost    */
  double comm_size = xbt_str_parse_double(argv[3], "Invalid communication size: %s");  /** - Task communication size */
  long edge_nodes_count = xbt_str_parse_int(argv[4], "Invalid amount of edge nodes: %s");    /** - Number of edge nodes */

  int i;

  XBT_INFO("Got %ld edge nodes and %ld tasks to process", edge_nodes_count, number_of_tasks);

  for (i = 0; i < number_of_tasks; i++) {  /** For each task to be executed: */
    char mailbox[256];
    char task_name[256];

    sprintf(mailbox, "edge-node-%ld", i % edge_nodes_count); /** - Select a @ref edge node in a round-robin way */
    sprintf(task_name, "Task_%d", i);
    msg_task_t task = MSG_task_create(task_name, comp_size, comm_size, NULL);   /** - Create a task */
    if (number_of_tasks < 10000 || i % 10000 == 0)
      XBT_INFO("Sending \"%s\" (of %ld) to mailbox \"%s\"", task->name, number_of_tasks, mailbox);

    MSG_task_send(task, mailbox); /** - Send the task to the @ref edge node */
  }

  XBT_INFO("All tasks have been dispatched. Let's tell everybody the computation is over.");
  for (i = 0; i < edge_nodes_count; i++) { /** - Eventually tell all the edge nodes to stop by sending a "finalize" task */
    char mailbox[80];

    sprintf(mailbox, "edge-node-%ld", i % edge_nodes_count);
    msg_task_t finalize = MSG_task_create("finalize", 0, 0, 0);
    MSG_task_send(finalize, mailbox);
  }

  return 0;
}

static int edge_node(int argc, char *argv[])
{
  msg_task_t task = NULL;
  char mailbox[80];

  long id= xbt_str_parse_int(argv[1], "Invalid argument %s");

  sprintf(mailbox, "edge-node-%ld", id); /** - unique id of the edge node */

  while (1) {  /** The edge node wait in an infinite loop for tasks sent by the \ref cloud */
    int res = MSG_task_receive(&(task), mailbox);
    xbt_assert(res == MSG_OK, "MSG_task_get failed");

    XBT_INFO("Received \"%s\"", MSG_task_get_name(task));
    if (!strcmp(MSG_task_get_name(task), "finalize")) {
      MSG_task_destroy(task);  /** - Exit if 'finalize' is received */
      break;
    }

    XBT_INFO("Processing \"%s\"", MSG_task_get_name(task));
    MSG_task_execute(task);    /**  - Otherwise, process the task */

    XBT_INFO("\"%s\" done", MSG_task_get_name(task));
    MSG_task_destroy(task);
    task = NULL;
  }
  XBT_INFO("I'm done. See you!");
  return 0;
}

int main(int argc, char *argv[])
{
  MSG_init(&argc, argv);
  xbt_assert(argc > 2, "Usage: %s platform_file deployment_file\n"
             "\tExample: %s msg_platform.xml msg_deployment.xml\n", argv[0], argv[0]);

  MSG_create_environment(argv[1]);          /** - Load the platform description */

  MSG_function_register("edge_node", edge_node);  /** - Register the function to be executed by the processes */
  MSG_function_register("cloud", cloud);
  MSG_launch_application(argv[2]);          /** - Deploy the application */

  msg_error_t res = MSG_main();             /** - Run the simulation */

  XBT_INFO("Simulation time %g", MSG_get_clock());

  return res != MSG_OK;
}
