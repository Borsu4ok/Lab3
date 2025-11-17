#include <iostream>
#include <thread>
#include <latch>
#include <syncstream>
#include <vector>
#include <functional>

struct SyncContext {
    std::latch latch_a{ 6 };
    std::latch latch_b{ 6 };
    std::latch latch_c{ 5 };
    std::latch latch_d{ 6 };
    std::latch latch_e{ 4 };
    std::latch latch_f{ 7 };
    std::latch latch_g{ 8 };
    std::latch latch_h{ 6 };
    std::latch latch_i{ 6 };
    std::latch latch_j{ 6 };
};

void f(char set, int i, std::ostream& out) {
    std::osyncstream(out) << "From set " << set << " action " << i << " completed." << std::endl;
}

void run_actions(char set, int from, int to, std::ostream& out, std::latch& l) {
    for (int i = from; i <= to; ++i) {
        f(set, i, out);
        l.count_down();
    }
}

void worker_1(std::ostream& out, SyncContext& ctx) {
    run_actions('a', 1, 6, out, ctx.latch_a);

    ctx.latch_b.wait();
    run_actions('e', 1, 4, out, ctx.latch_e);

    ctx.latch_d.wait();
    ctx.latch_g.wait();
    ctx.latch_h.wait();
    run_actions('j', 1, 2, out, ctx.latch_j);
}

void worker_2(std::ostream& out, SyncContext& ctx) {
    run_actions('b', 1, 6, out, ctx.latch_b);

    ctx.latch_b.wait();
    run_actions('f', 1, 6, out, ctx.latch_f);
}

void worker_3(std::ostream& out, SyncContext& ctx) {
    run_actions('c', 1, 5, out, ctx.latch_c);

    ctx.latch_c.wait();
    run_actions('g', 1, 7, out, ctx.latch_g);
}

void worker_4(std::ostream& out, SyncContext& ctx, std::latch& main_latch) {
    run_actions('d', 1, 6, out, ctx.latch_d);

    ctx.latch_c.wait();
    run_actions('h', 1, 6, out, ctx.latch_h);

    ctx.latch_i.wait();
    main_latch.count_down();
}

void worker_5(std::ostream& out, SyncContext& ctx, std::latch& main_latch) {
    ctx.latch_b.wait();
    run_actions('f', 7, 7, out, ctx.latch_f);

    ctx.latch_c.wait();
    run_actions('g', 8, 8, out, ctx.latch_g);

    ctx.latch_a.wait();
    ctx.latch_e.wait();
    ctx.latch_f.wait();
    run_actions('i', 1, 6, out, ctx.latch_i);

    ctx.latch_d.wait();
    ctx.latch_g.wait();
    ctx.latch_h.wait();
    run_actions('j', 3, 6, out, ctx.latch_j);

    ctx.latch_j.wait();
    main_latch.count_down();
}

int main() {
    std::osyncstream(std::cout) << "Calculation started." << std::endl;

    SyncContext context;
    std::latch main_latch(2);
    std::vector<std::jthread> threads;

    threads.emplace_back(worker_1, std::ref(std::cout), std::ref(context));
    threads.emplace_back(worker_2, std::ref(std::cout), std::ref(context));
    threads.emplace_back(worker_3, std::ref(std::cout), std::ref(context));
    threads.emplace_back(worker_4, std::ref(std::cout), std::ref(context), std::ref(main_latch));
    threads.emplace_back(worker_5, std::ref(std::cout), std::ref(context), std::ref(main_latch));

    main_latch.wait();

    std::osyncstream(std::cout) << "Calculation finished." << std::endl;

    return 0;
}