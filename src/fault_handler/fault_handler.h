#pragma once

void fault_handler(int, siginfo_t*, void*);
void configure_sig_handler(void (*)(int, siginfo_t*, void*));