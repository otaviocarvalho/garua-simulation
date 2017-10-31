#include "simgrid/msg.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(msg_simulation, "Messages specific for this msg example");

static int edge_node(int argc, char *argv[])
{
    int edge_node_id = xbt_str_parse_int(argv[1], "Invalid edge node id: %s");    /** - Edge node id     */
    long number_of_tasks = xbt_str_parse_int(argv[2], "Invalid amount of tasks: %s");    /** - Number of tasks      */
    double comp_size = xbt_str_parse_double(argv[3], "Invalid computational size: %s");  /** - Task compute cost    */
    double comm_size = xbt_str_parse_double(argv[4], "Invalid communication size: %s");  /** - Task communication size */
    long cloud_nodes_count = xbt_str_parse_double(argv[5], "Invalid amount of cloud nodes: %s");  /** - Cloud nodes count */
    long edge_nodes_count = xbt_str_parse_double(argv[6], "Invalid amount of edge nodes: %s");  /** - Edge nodes count */

    int i;
    int task_slice_size = number_of_tasks / edge_nodes_count;
    int slice_start = (edge_node_id % number_of_tasks) * task_slice_size;
    int slice_end = slice_start + task_slice_size;
    XBT_INFO("### slice start %d : slice end %d", slice_start, slice_end);

    for (i = slice_start; i < slice_end; i++) {  /** For each task to be executed: */
        char mailbox[256];
        char task_name[256];

        sprintf(mailbox, "cloud-0"); /** - Select a @ref cloud node in a round-robin way */
        sprintf(task_name, "Task_%d", i);
        msg_task_t task = MSG_task_create(task_name, comp_size, comm_size, NULL);   /** - Create a task */

        XBT_INFO("Sending \"%s\" (of %ld) to mailbox \"%s\"", task->name, number_of_tasks, mailbox);

        MSG_task_send(task, mailbox); /** - Send the task to the @ref edge node */

        task = NULL;
    }

    return 0;
}

static int cloud(int argc, char *argv[])
{
    long number_of_tasks = xbt_str_parse_int(argv[1], "Invalid amount of tasks: %s");    /** - Number of tasks      */
    long edge_nodes_count = xbt_str_parse_int(argv[2], "Invalid amount of edge nodes: %s");    /** - Number of edge nodes */

    XBT_INFO("Got %ld edge nodes and a total of %ld tasks to process", edge_nodes_count, number_of_tasks);

    msg_task_t task = NULL;
    char mailbox[80];

    sprintf(mailbox, "cloud-0"); /** - unique id of the cloud node */

    int num_finalized_tasks = 0;
    while (1) {  /** The cloud node wait in an infinite loop for tasks sent by \ref edge nodes */
        int res = MSG_task_receive(&(task), mailbox);
        xbt_assert(res == MSG_OK, "MSG_task_get failed");

        XBT_INFO("Processing \"%s\"", MSG_task_get_name(task));
        MSG_task_execute(task);    /**  - Otherwise, process the task */

        XBT_INFO("\"%s\" done", MSG_task_get_name(task));
        MSG_task_destroy(task);
        task = NULL;

        num_finalized_tasks++;

        // Finish when has received all edge node tasks
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
