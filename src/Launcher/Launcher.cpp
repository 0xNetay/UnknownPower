//
// Created by student on 2/29/20.
//

#include "Launcher.hpp"

extern IgnoredPrefix prefix_blacklist[MAX_BLACKLISTED_PREFIXES];
extern IgnoredOpcode opcode_blacklist[MAX_BLACKLISTED_OPCODES];
extern Config global_config;
extern RegState injected_reg_state;

Launcher::Launcher(int argc, char** argv) : _argc(argc), _argv(), _instruction_manager(&_pool_mutex)
{
}

bool Launcher::Init()
{
    pthread_mutexattr_init(&this->_mutex_attr);
    pthread_mutexattr_setpshared(&this->_mutex_attr, PTHREAD_PROCESS_SHARED);

    this->_pool_mutex = reinterpret_cast<pthread_mutex_t*>(
        mmap(nullptr, sizeof(pthread_mutex_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0));

    this->_output_mutex = reinterpret_cast<pthread_mutex_t*>(
        mmap(nullptr, sizeof(pthread_mutex_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0));

    pthread_mutex_init(this->_pool_mutex, &this->_mutex_attr);
    pthread_mutex_init(this->_output_mutex, &this->_mutex_attr);

    OutputManager::Instance().SetOutputMode(OutputMode::Raw);
    OutputManager::Instance().SetOutputMutex(_output_mutex);

    if (!this->Configure())
    {
        return false;
    }

    ProcessorManager::PinCore();

    srand(ConfigManager::Instance().GetConfig().seed_for_random);

    static char stack[SIGSTKSZ] = { 0 };
    static stack_t ss = { stack, 0, SIGSTKSZ };
    sigaltstack(&ss, nullptr);

    this->_packet_buffer_unaligned = malloc(PAGE_SIZE*3);
    *ProcessorManager::GetMutablePacketBuffer() = reinterpret_cast<char*>(((uintptr_t)this->_packet_buffer_unaligned + (PAGE_SIZE - 1)) & ~(PAGE_SIZE-1));
    OutputManager::Instance().SetPacketBuffer(*ProcessorManager::GetMutablePacketBuffer());

    assert(!mprotect(*ProcessorManager::GetMutablePacketBuffer(), PAGE_SIZE, PROT_READ|PROT_WRITE|PROT_EXEC));
    if (ConfigManager::Instance().GetConfig().no_execute_support)
    {
        assert(!mprotect(reinterpret_cast<char*>(*ProcessorManager::GetMutablePacketBuffer()) + PAGE_SIZE, PAGE_SIZE, PROT_READ|PROT_WRITE));
    }
    else
    {
        assert(!mprotect(reinterpret_cast<char*>(*ProcessorManager::GetMutablePacketBuffer()) + PAGE_SIZE, PAGE_SIZE, PROT_NONE));
    }

#if USE_CAPSTONE
    if (cs_open(CS_ARCH_X86, CS_MODE, &this->_capstone_handle) != CS_ERR_OK)
    {
        exit(1);
    }
    this->_capstone_insn = cs_malloc(this->_capstone_handle);

    OutputManager::Instance().SetCapstoneHandler(this->_capstone_handle);
    OutputManager::Instance().SetCapstoneInstructions(this->_capstone_insn);

    // OutputManager::Instance().GetMutableDisassemblyInfo() = _disassembly_info;
#endif

    if (ConfigManager::Instance().GetConfig().enable_null_access)
    {
        this->_null_p = mmap(0, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        if (this->_null_p == MAP_FAILED)
        {
            printf("null access requires running as root\n");
            exit(1);
        }
    }

    this->_instruction_manager.CreateRanges();

    return true;
}

void Launcher::Run()
{
    int pid = 0;
    for (size_t i = 0; i < ConfigManager::Instance().GetConfig().jobs_count - 1; i++)
    {
        pid = fork();
        assert(pid >= 0);
        if (pid == 0)
        {
            break;
        }
    }

    // Main Loop
    while (this->_instruction_manager.BuildNextRange())
    {
        while (this->_instruction_manager.BuildNextInstruction())
        {
            Instruction current = this->_instruction_manager.GetCurrentInstruction();
            Launcher::PrintLoopStatus(current);

            size_t i;
            for (i = MIN_INSTRUCTION_LENGTH; i <= MAX_INSTRUCTION_LENGTH; i++)
            {
                current.length = i;
                ProcessorManager::InjectInstructionToProcessor(current);
                if (OutputManager::Instance().GetImmutableResult().address !=
                    (uint32_t)(uintptr_t)(*ProcessorManager::GetMutablePacket() + PAGE_SIZE))
                {
                    break;
                }
            }

            OutputManager::Instance().GetMutableResult().length = i;
            OutputManager::Instance().GiveResultToOutput(current, this->_instruction_manager.GetBuildMode(), stdout);
            OutputManager::Instance().Tick(current, this->_instruction_manager.GetBuildMode());
        }
    }

    if (pid != 0)
    {
        for (size_t i = 0; i < ConfigManager::Instance().GetConfig().jobs_count - 1; i++)
        {
            wait(NULL);
        }
    }
}

void Launcher::Close()
{
    OutputManager::Instance().SyncFlushOutput(stdout, true);
    OutputManager::Instance().SyncFlushOutput(stderr, true);

    this->_instruction_manager.DropRanges();
    pthread_mutex_destroy(this->_pool_mutex);
    pthread_mutex_destroy(this->_output_mutex);

#if USE_CAPSTONE
    cs_free(this->_capstone_insn, 1);
    cs_close(&this->_capstone_handle);
#endif

    if (ConfigManager::Instance().GetConfig().enable_null_access)
    {
        munmap(this->_null_p, PAGE_SIZE);
    }

    free(this->_packet_buffer_unaligned);
}

bool Launcher::Configure()
{
    Config config = global_config;

    int c;
    opterr = 0;
    bool seed_given=false;

    while ((c = getopt(_argc, _argv, "?brtdRTx0Ns:DB:P:S:c:X:j:l:")) != -1)
    {
        switch (c)
        {
            case '?':
                Launcher::PrintHelp();
                exit(-1);

            case 'b':
                _instruction_manager.SetBuildMode(BuildMode::BruteForce);
                break;

            case 'r':
                _instruction_manager.SetBuildMode(BuildMode::Random);
                break;

            case 't':
            case 'd':
                _instruction_manager.SetBuildMode(BuildMode::TunnelMinMax);
                break;

            case 'R':
                OutputManager::Instance().SetOutputMode(OutputMode::Raw);
                break;

            case 'T':
                OutputManager::Instance().SetOutputMode(OutputMode::Text);
                break;

            case 'x':
                config.should_show_tick = true;
                break;

            case '0':
                config.enable_null_access = true;
                break;

            case 'N':
                config.no_execute_support = false;
                break;

            case 's':
                sscanf(optarg, "%ld", &config.seed_for_random);
                seed_given = true;
                break;

            case 'P':
                sscanf(optarg, "%zd", &config.max_explored_prefix);
                break;

            case 'B':
                sscanf(optarg, "%zd", &config.brute_force_byte_depth);
                break;

            case 'D':
                config.allow_exploring_more_than_one_prefix_in_search = true;
                break;

            case 'c':
                config.force_core = true;
                sscanf(optarg, "%zd", &config.core_count);
                break;

            case 'X':
                {
                    size_t j = 0;
                    while (opcode_blacklist[j].opcode)
                    {
                        j++;
                    }

                    opcode_blacklist[j].opcode = static_cast<char*>(malloc(strlen(optarg) / 2 + 1));
                    assert (opcode_blacklist[j].opcode);

                    size_t i = 0;
                    while (optarg[i*2] && optarg[i*2+1])
                    {
                        unsigned int k;
                        sscanf(optarg+i*2, "%02x", &k);
                        opcode_blacklist[j].opcode[i] = char(k);
                        i++;
                    }

                    opcode_blacklist[j].opcode[i] = '\0';
                    opcode_blacklist[j].reason = "user_blacklist";
                    opcode_blacklist[++j] = {NULL,NULL};
                }
                break;

            case 'j':
                sscanf(optarg, "%zd", &config.jobs_count);
                break;

            case 'l':
                sscanf(optarg, "%zd", &config.range_bytes);
                break;

            default:
                Launcher::PrintUsage();
                exit(-1);
        }
    }

    if (optind != _argc)
    {
        Launcher::PrintUsage();
        exit(1);
    }

    if (!seed_given)
    {
        config.seed_for_random = time(nullptr);
    }

    ConfigManager::Instance().SetConfig(config);
    ConfigManager::Instance().SetRegState(injected_reg_state);

    size_t opcode_blacklist_count = 0;
    while (opcode_blacklist[opcode_blacklist_count].opcode)
    {
        opcode_blacklist_count++;
    }
    ConfigManager::Instance().SetOpcodeBlackList(opcode_blacklist, opcode_blacklist_count + 1);

    size_t prefix_blacklist_count = 0;
    while (prefix_blacklist[prefix_blacklist_count].prefix)
    {
        prefix_blacklist_count++;
    }
    ConfigManager::Instance().SetPrefixBlackList(prefix_blacklist, prefix_blacklist_count + 1);

    return true;
}

void Launcher::PrintHelp()
{
    printf("injector [OPTIONS...]\n");
    printf("\t[-b|-r|-t|-d] ....... mode: brute, random, tunnel, directed (default: tunnel)\n");
    printf("\t[-R|-T] ............. output: Raw, Text (default: Text)\n");
    printf("\t[-x] ................ show tick (default: %d)\n", ConfigManager::Instance().GetConfig().should_show_tick);
    printf("\t[-0] ................ allow null dereference (requires sudo) (default: %d)\n", ConfigManager::Instance().GetConfig().enable_null_access);
    printf("\t[-D] ................ allow duplicate prefixes (default: %d)\n", ConfigManager::Instance().GetConfig().allow_exploring_more_than_one_prefix_in_search);
    printf("\t[-N] ................ no nx bit support (default: %d)\n", ConfigManager::Instance().GetConfig().no_execute_support);
    printf("\t[-s seed] ........... in random search, seed (default: time(0))\n");
    printf("\t[-B brute_depth] .... in brute search, maximum search depth (default: %zu)\n", ConfigManager::Instance().GetConfig().brute_force_byte_depth);
    printf("\t[-P max_prefix] ..... maximum number of prefixes to search (default: %zu)\n", ConfigManager::Instance().GetConfig().max_explored_prefix);
    printf("\t[-c core] ........... core on which to perform search (default: any)\n");
    printf("\t[-X blacklist] ...... blacklist the specified instruction\n");
    printf("\t[-j jobs] ........... number of simultaneous jobs to run (default: %zu)\n", ConfigManager::Instance().GetConfig().jobs_count);
    printf("\t[-l range_bytes] .... number of base instruction bytes in each sub range (default: %zu)\n", ConfigManager::Instance().GetConfig().range_bytes);
}

void Launcher::PrintUsage()
{
    printf("injector [-b|-r|-t|-d] [-R|-T] [-x] [-0] [-D] [-N]\n");
    printf("\t[-s seed] [-B brute_depth] [-P max_prefix]\n");
    printf("\t[-i instruction] [-e instruction]\n");
    printf("\t[-c core] [-X blacklist]\n");
    printf("\t[-j jobs] [-l range_bytes]\n");
}

void Launcher::PrintLoopStatus(const Instruction& instruction)
{
    if (OutputManager::Instance().GetOutputMode() == OutputMode::Text)
    {
        OutputManager::Instance().SyncPrintFormat(stdout, "r: ");
        OutputManager::Instance().PrintInMcToOutput(instruction, stdout);
        OutputManager::Instance().SyncPrintFormat(stdout, "... ");

#if USE_CAPSTONE
        OutputManager::Instance().PrintInstructionInAsmToOutput(instruction, stdout);
        OutputManager::Instance().SyncPrintFormat(stdout, " ");
#endif

        OutputManager::Instance().SyncFlushOutput(stdout, false);
    }
}