---
finished_date: 2020-11-25
tags:
    - OS
    - virtual_memory
    - page_replacement
    - C
    - linux
---
# Deadlock Detection with Graph Reduction method
- Implement virtual memory management method in C
- give the number of page fault, residence set, victim
- Used Algorithm: MIN, FIFO, LFU, LRU, Clock, Working Set(WS)
## input file
```
A B C D
s<sub>0</sub> s<sub>1</sub> s<sub>2</sub> s<sub>3</sub> ...
```
    - A: the number of page(max 100, start number = 0)
    - B: the number of page frame to be allocated (max 20, not used in WS algorithm) 
    - C: window size (max 100, only used in WS algorithm)
    - D: length of page reference string (max 100,000)
    - s<sub>0</sub>: page reference string
## output file

![image](https://user-images.githubusercontent.com/74483608/160330696-3e6d8ac0-c0e6-4ddd-b158-5505383892af.png)

    - output.txt
        - for each algirhtm
        - when page fault occured, time, replaced index is printed
        - show residence set for each time
        - show total number of page fault occured
## Solving strategy

![block diagram](https://user-images.githubusercontent.com/74483608/160331160-d9c139b3-4d29-440d-a9e7-417856c08d3f.png)

- MIN: replace a page has longest forward distance
- FIFO: replace a page whose arrival time is smallest
- LRU: replace a page whose used time is minimal
- LFU: replace a page whose used count is minimum
- clock: replace a page whose reference bit is 0
- WS: kind of variable allocation, expel page whose used time is not in window

![array 설명](https://user-images.githubusercontent.com/74483608/160331211-06590e69-b369-4f50-bb1e-2434b6a239ad.png)


- detailed implementation information is included in src code as comment

- random input function is added
- error cases of improper input file are also tested using error*.txt

## Result
예시

![image](https://user-images.githubusercontent.com/74483608/160332460-f9fb7a13-8317-4f72-89a1-08f45c5b2bdd.png)

## File structure
```
|-- data
    |-- input.txt
    |-- input1.txt
    |-- input2.txt
    |-- input3.txt
    |-- input4.txt
    |-- input5.txt
    |-- input6.txt
    |-- input7.txt
    |-- input8.txt
    |-- input9.txt
    |-- input10.txt
    |-- input11.txt
    |-- error1.txt
    |-- error2.txt
    |-- error3.txt
    |-- error4.txt
    |-- error5.txt
    |-- rand_input.txt  // example input file of random input mode
|-- src
    |-- virtual_memory_management.c
|-- virtual_memory_management_docuemnt.pdf
|-- output.txt
```
## 배운 점
- the number of page >> the number of frame 일 때 page fault ratio가 매우 높다. 그러나 WS algorithm의 경우, page fault ratio가 window size에 의해 영향을 받기 때문에 다른 algorithm에 비해 page fault가 적다.
- the number of page < the number of frame일 때 WS를 제외한 알고리즘은 처음 page를 load할 때만 fault가 발생한느 반면 WS는 working set만 residence set이 되기 때문에 FA보다 fault가 많이 발생한다.
- WS에서 window size가 증가하면 fault 감소, 평균 frame 수 증가인 반면 window size가 감소되면 fault 증가, 평균 frame 수 감소가 나타난다.
- error handling을 진행하여 이전 프로젝트와 비교하였을 때 시스템의 안정성을 높였다.
## 한계점
- header 파일로 구현한 함수 혹은 자료구조를 처리하여 file을 나눠 관리하였다면 재사용성, 가독성과 관리의 측면에서 도움이 되었을 것이다.
