// Requesting all resources
// 철학자가 포크 두 개를 '동시에' 집을 수 있을 때만 집도록 글로벌 락 활용
pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t forks[5];

void* philosopher_all_res(void* arg) {
    int id = *(int*)arg;
    int left = id;
    int right = (id + 1) % 5;

    // 두 개의 자원을 한 번에 요청
    pthread_mutex_lock(&global_lock);
    pthread_mutex_lock(&forks[left]);
    pthread_mutex_lock(&forks[right]);
    pthread_mutex_unlock(&global_lock);

    // Eating
    printf("Philosopher %d is eating.\n", id);

    pthread_mutex_unlock(&forks[left]);
    pthread_mutex_unlock(&forks[right]);
    return NULL;
}

//Resource ordering
// 홀수/짝수 철학자의 행동을 다르게 하여 순환 대기(Circular Wait) 조건 파괴
void* philosopher_ordering(void* arg) {
    int id = *(int*)arg;
    int left = id;
    int right = (id + 1) % 5;

    if (id % 2 == 0) { // 짝수: 왼쪽 먼저
        pthread_mutex_lock(&forks[left]);
        pthread_mutex_lock(&forks[right]);
    }
    else {           // 홀수: 오른쪽 먼저
        pthread_mutex_lock(&forks[right]);
        pthread_mutex_lock(&forks[left]);
    }

    // Eating
    printf("Philosopher %d is eating.\n", id);

    pthread_mutex_unlock(&forks[left]);
    pthread_mutex_unlock(&forks[right]);
    return NULL;
}

/* Banker's Algorithm 개념적 적용:
- Available: 현재 식탁에 놓인 포크 배열 [1,1,1,1,1]
- Max Need: 각 철학자는 인접한 2개의 포크가 필요
포크를 요청할 때마다 시스템이 안전 상태인지 확인하는 함수를 거칩니다.
*/
pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
int state[5] = { 0 }; // 0: Thinking, 1: Hungry, 2: Eating

void test(int i) {
    // 양쪽 철학자가 밥을 먹고 있지 않고 내가 배고프면 식사 가능 
    if (state[i] == 1 && state[(i + 4) % 5] != 2 && state[(i + 1) % 5] != 2) {
        state[i] = 2; // Eating
        // Condition Variable signal() 로 대기중인 철학자 깨움
    }
}
// 픽업 시 state를 1로 변경 후 test() 실행, 못 먹으면 wait()
// 풋다운 시 state를 0으로 변경 후 양쪽 철학자에 대해 test() 실행