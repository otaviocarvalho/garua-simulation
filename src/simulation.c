#include "simgrid/msg.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(msg_simulation, "Messages specific for this msg example");

static int edge_node(int argc, char *argv[])
{
    int edge_node_id = xbt_str_parse_int(argv[1], "Invalid edge node id: %s");    /** - Edge node id     */
    long number_of_tasks = xbt_str_parse_int(argv[2], "Invalid amount of tasks: %s");    /** - Number of tasks      */
    double comp_size = xbt_str_parse_double(argv[3], "Invalid computational size: %s");  /** - Task compute cost    */
    double comm_size = xbt_str_parse_double(argv[4], "Invalid communication size: %s");  /** - Task communication size */

    int i;
    for (i = 0; i < number_of_tasks; i++) {  /** For each task to be executed: */
        char mailbox[256];
        char task_name[256];

        sprintf(mailbox, "cloud-%d", edge_node_id); // Send a message to the corresponding cloud node (simulating a thread on the cloud node per edge node)
        sprintf(task_name, "Task_%d", i);
        msg_task_t task = MSG_task_create(task_name, comp_size, comm_size, NULL);   /** - Create a task */

        XBT_INFO("Sending \"%s\" (of %ld) to mailbox \"%s\"", task->name, number_of_tasks, mailbox);

        MSG_task_send(task, mailbox); /** - Send the task to the @ref edge node */

        task = NULL;
    }

    XBT_INFO("All tasks have been dispatched. Finishing edge node, bye!");

    return 0;
}

static int cloud(int argc, char *argv[])
{
    int cloud_node_id = xbt_str_parse_int(argv[1], "Invalid cloud node id: %s");    /** - Cloud node id */
    long number_of_tasks = xbt_str_parse_int(argv[2], "Invalid amount of tasks: %s");    /** - Number of tasks      */

    XBT_INFO("Cloud node id %d got a total of %ld tasks to process", cloud_node_id, number_of_tasks);

    msg_task_t task = NULL;
    char mailbox[80];

    sprintf(mailbox, "cloud-%d", cloud_node_id); /** - unique id of the cloud node */

    int num_finalized_tasks = 0;
    while (1) {  /** The cloud node wait in an infinite loop for tasks sent by \ref edge nodes */
        int res = MSG_task_receive(&(task), mailbox);
        xbt_assert(res == MSG_OK, "MSG_task_get failed");

        XBT_INFO("Processing \"%s\"", MSG_task_get_name(task));
        MSG_task_execute(task);

        XBT_INFO("\"%s\" done", MSG_task_get_name(task));
        MSG_task_destroy(task);
        task = NULL;

        num_finalized_tasks++;

        if (num_finalized_tasks == number_of_tasks) {
            XBT_INFO("Number of tasks completed: %ld", number_of_tasks);
            XBT_INFO("Finishing cloud node, bye!");
            break;
        }
    }

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
