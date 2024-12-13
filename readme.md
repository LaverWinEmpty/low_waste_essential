Thread-safe memory pool

-   만들어놓고 여기저기 쓰려고 외부로 뺐습니다

-   현재 Windows에서만 동작하게 만들어져있습니다.

-   블럭을 노드 단위로 생성하고 자신의 블럭 위치를 기억합니다. 즉 미사용 블럭은 자유롭게 지울 수 있습니다.

-   lock-free queue와 thread_local을 사용하여 multi-thread 환경에서 빠르고 안전하게 동작합니다.

작성중
