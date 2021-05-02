# 숭실대학교 _ 리눅스 시스템 프로그래밍 개인프로젝트 P3 _ 동기화 && 비동기화

### 목차
- [설계 및 구현](#설계-및-구현)
- [소스코드 분석](#소스코드-분석)

## 설계 및 구현

- ssu_crontab 프로그램 기본 사항
  
  - 사용자가 주기적으로 실행하는 명령어를 “ssu_crontab_file”에 저장 및 삭제하는 프로그램
  - 주기적으로 “ssu_crontab_file”에 저장된 명령어를 실행시킬 “ssu_crond” 디몬 프로그램
    
    - “ssu_crond”는 운영체제 시작 시 함께 실행되어 “ssu_crontab_file”에 저장된 명령어를 주기적으로 실행시킴
  
  - “ssu_crontab_file”에 저장된 명령어가 정상적으로 수행된 경우만 “ssu_crontab_log”로그파일에 다음 사항을 기록
    
    ![1](https://user-images.githubusercontent.com/47939832/112301356-32085500-8cdd-11eb-8763-33d7cb3a8f76.png)
    
    - 출력형태
      
      - [수행시간] 프롬프트명령어 실행주기 명령어 명령어 옵션
      - [수행시간] 프롬프트명령어 실행주기 명령어 명령어 옵션
      - 프롬프트 명령어 : add, remove, run 중 하나표시
        
        - 프롬프트 명령어 : add, remove, run 중 하나표시
        - remove : ssu_crontab을 통해 명령어가 삭제된 경우
        - run : ssu_crond를 통해 명령어가 수행된 경우
      
      - 실행주기 : 분 시 일 월 요일
        
        - 각 항목의 값으로 들어가는 내용은 설계 및 구현에서 설명
      
      - 명령어와 명령어 옵션
    
    - “ssu_crontab_file”에 명령어가 추가, 제거 될 때마다 “ssu_crontab_log”로그파일에 기록
    - “ssu_crond”가 “ssu_crontab_file”에 저장된 명령어를 주기적으로 실행할 때마다 “ssu_crontab_log”로그파일에 기록

- ssu_rsync 프로그램 기본 사항
  
  - 명령어 형태 : ssu_rsync src(소스파일 또는 디렉토리) dst(목적 디렉토리)
  - 인자로 주어진 src 파일 혹은 디렉토리를 dst 디렉토리에 동기화
  - 동기화 시 src 파일이 dst 디렉토리 내 동일한 파일(파일 이름과 파일 크기, 수정시간이 같은 경우)로 존재할 경우 해당 파일은 동기화 하지 않음
  - 예1) src가 일반 파일인 경우 src와 동일한 이름의 dst 디렉토리 내에 일반파일로 존재할 경우 해당 파일은 동기화 하지 않음
  - 예2) src가 디렉토리 파일인 경우 src 디렉토리 내 모든 파일과 dst 디렉토리 내 모든 파일을 비교하여 동일한 일반 파일이 존재할 경우 해당 파일은 동기화하지 않음
  - 동기화 작업 도중 SIGINT가 발생하면 동기화 작업이 취소되고 dst 디렉토리 내 파일들이 동기화하지 않은 상태로 유지되어야 함
  - 단 동기화 중에는 사용자가 동기화 중인 파일을 open 하는 것은 허용하지 않음
  - src 파일 dst 디렉토리 내 파일의 동기화가 모두 완료되면 “ssu_rsync_log”로그파일에 기록
    
    - 출력형태
      
      - [동기화 완료시간] 명령어 및 명령어 옵션 파일 이름 파일 크기
      - 동기화 완료시간 : 요일 월 날짜 명령어 동기화 완료시간
      - 명령어 및 명령어 옵션
      - 파일 이름 : 동기화한 파일의 이름(src 디렉토리 기준 상대경로)
      - 파일 크기 : 동기화한 파일의 크기, 단. 파일이 삭제된 경우 delete를 기록
        
        ![2](https://user-images.githubusercontent.com/47939832/112301363-33398200-8cdd-11eb-93b3-0d320352ed47.png)
        
- 가) ssu_crontab

  - ssu_crontab 실행 후 다음과 같은 프롬프트 출력
    
    - 프롬프트 모양 : “ssu_crontab_file”에 저장된 모든 명령어 출력 및 개행 후, 공백 없이 학번, ‘>’ 문자 출력
      
      ![3](https://user-images.githubusercontent.com/47939832/112301364-33d21880-8cdd-11eb-99dd-15129670412f.png)
      
    - 프롬프트에서 실행 가능한 명령어 : add, remove, exit
    - 이외 명령어 입력 시 에러 처리 후 프롬프트로 제어가 넘어감
    - 엔터만 입력 시 프롬프트 재출력
    - exit 명령이 입력될 때까지 위에서 지정한 실행 가능 명령어를 입력받아 실행
    
  - add <실행주기> <명령어>
    
    - “ssu_crontab_file”에 실행주기와 명령어가 기록되어야 하며 “ssu_crontab_file”파일이 없을 경우 생성함
    - 실행주기
      
      - 실행주기의 각 항목은 분 시 일 월 요일 의 5가지 항목으로 구성됨
      - 각 항목은 분(0-59), 시(0-23), 일(1-31), 월(1-12), 요일(0-6, 0은 일요일 1~6순으로 월요일에서 토요일을 의미)의 범위를 가짐
      - 각 항목의 값은 ‘*’, ‘-’, ‘,’, ‘/’ 기호를 사용하여 설정할 수 있음
        
        - ‘*’: 해당 필드의 모든 값을 의미함
        - ‘-’: ‘-’으로 연결된 값 사이의 모든 값을 의미함(범위 지정)
        - ‘-’: ‘-’으로 연결된 값 사이의 모든 값을 의미함(범위 지정)
        - ‘/’: 앞에 나온 주기의 범위를 뒤에 나온 숫자만큼 건너뛰는 것을 의미함
        
      - 예) 0 0,12 */2 * * -> 2일마다 0시 0분, 12시 0분에 작업수행
      
    - 명령어 인자에는 주기적으로 수행할 명령어 및 해당 명령어의 옵션까지 모두 입력
    - 실행주기의 입력이 잘못 된 경우 에러 처리 후 프롬프트로 제어가 넘어감
    - add 프롬프트 명령어를 통해 명령어가 “ssu_crontab_file”파일에 저장되면 “ssu_crontab_log”로그파일에 로그를 남김
      
      ![4](https://user-images.githubusercontent.com/47939832/112301366-346aaf00-8cdd-11eb-88db-ea68ad9681d6.png)
  
  - remove <COMMAND_NUMBER>
    
    - 옵션으로 입력한 번호의 명령어를 제거
    - 잘못된 COMMAND_NUMBER가 입력된 경우 에러 처리 후 프롬프트로 제어가 넘어감
    - COMMAND_NUMBER를 입력하지 않은 경우 에러 처리 후 프롬프트로 제어가 넘어감
    - remove 프롬프트 명령어를 통해 명령어가 “ssu_crontab_file”파일에서 삭제되면 “ssu_crontab_log”로그파일에 로그를 남김
      
      ![5](https://user-images.githubusercontent.com/47939832/112301368-346aaf00-8cdd-11eb-8832-4bf52d4a71ac.png)
      
  - exit
    
    - 프로그램 종료
      

- 나) ssu_crond

  - 운영체제 시작 시 함께 실행되어 “ssu_crontab_file”에 저장된 명령어를 주기적으로 실행시키는 디몬 프로그램
  - “ssu_crond”는 “ssu_crontab_file”을 읽어 주기적으로 명령어를 실행해야 함
  - “ssu_crond”는 “ssu_crontab_file”을 읽어 주기적으로 명령어를 실행해야 함
  - “ssu_crond”를 통해 명령어가 수행되면 “ssu_crontab_log”로그파일에 로그를 남김

- 다) ssu_rsync [option] <src> <dst>

  - “ssu_rsync”는 인자로 주어진 src 파일 혹은 디렉토리를 dst 디렉토리에 동기화함
  - 동기화 시 src 파일이 dst 디렉토리 내 동일한 파일(파일 이름과 파일 크기, 수정시간이 같은 경우)이 존재하지 않는 경우 src 파일을 dst 디렉토리 내에 복사함
  - 동기화 시 dst 디렉토리 내에 파일 이름이 같은 다른 파일(파일 이름은 같으나 파일 크기 혹은 수정시간이 다른 경우)이 존재할 경우 dst 디렉토리 내의 파일을 src 파일로 대체함
  - dst 디렉토리 내의 동기화된 파일은 “ssu_rsync”프로그램에서 src 디렉토리의 파일과 같은 파일로 인식할 수 있어야함
  - src 인자는 파일 및 디렉토리 모두 허용
    
    - 상대경로와 절대경로 모두 입력 가능
    - 인자로 입력받은 파일 혹은 디렉토리를 찾을 수 없으면 usage 출력 후 프로그램 종료
    - 인자로 입력받은 파일 혹은 디렉토리의 접근권한이 없는 경우 usage 출력 후 프로그램 종료
  
  - dst 인자는 디렉토리만 허용
    
    - 상대경로와 절대경로 모두 입력 가능
    - 인자로 입력받은 디렉토리를 찾을 수 없으면 usage 출력 후 프로그램 종료
    - 인자로 입력받은 디렉토리가 디렉토리 파일이 아니라면 usage 출력 후 프로그램 종료
    - 인자로 입력받은 디렉토리의 접근권한이 없는 경우 usage 출력 후 프로그램 종료
    
  - ‘-r’옵션을 설정하지 않은 경우 인자로 지정한 src의 서브디렉토리는 동기화하지 않음
  - ‘-m’옵션을 설정하지 않은 경우 src 디렉토리에 존재하지 않는 파일 및 디렉토리가 dst 디렉토리에 존재할 수 있음
  - 동기화 작업 도중 SIGINT가 발생하면 동기화 작업이 취소되고 dst 디렉토리 내 파일들이 동기화하지 않은 상태로 유지되어야 함

- 라) -r

  - -r 옵션이 설정되면 src의 서브디렉토리 내의 파일 또한 동기화함
  - dst 디렉토리에 동기화할 서브디렉토리가 없는 경우 디렉토리를 생성해 주어야함

- 마) -t

  - -t 옵션이 설정되면 동기화가 필요한 대상들을 묶어 한번에 동기화 작업을 수행
  - tar를 활용하여 묶음
  - tar파일 전송 완료 후 묶음 해제하여 정확한 위치에 동기화
  - -t 옵션 사용 시 로그기록 방식이 달라짐
  - 로그기록 방식
    
    [동기화 완료시간] 명령어 및 명령어 옵션
    
    totalSize 전송된 tar 파일의 크기
    
    파일이름
    
    - 동기화 완료시간 : 요일 월 날짜 명령어 동기화 완료시간
    - 명령어 및 명령어 옵션
    - totalSize : 로그 출력양식용 텍스트
    - 전송된 tar 파일의 크기 : 동기화가 필요한 대상들을 묶은 tar 파일의 크기
    - 파일 이름 : 동기화한 파일의 이름(src 디렉토리 기준 상대경로)

- 바) -m
  
  - -m 옵션이 설정되면 dst 디렉토리에 src 디렉토리에 없는 파일 및 디렉토리가 존재할 경우 dst 디렉토리에서 해당하는 파일 및 디렉토리를 삭제함
  - src 디렉토리에 없는 파일 및 디렉토리가 삭제되면 “ssu_rsync_log”로그파일에 로그를 남김


## 소스코드 분석

- ssu_crontab, ssu_crond
  
  ▶ 사용자가 주기적으로 실행하는 명령어를 “ssu_crontab_file”에 저장 및 삭제하는 프로그램이며 위 파일을 토대로 ssu_crond 디몬프로그램으로 주기적으로 명령어를 실행한다.

  ▶ ssu_crond 디몬 프로그램으로 정상적으로 실행된 내용은 “ssu_crontab_log” 로그파일에 기록을 남긴다.

  ▶ 또한, “ssu_crontab_file”에 명령어가 추가 및 제거 될 때마다 “ssu_crontab_log” 로그파일에 기록을 남긴다.
  
- ssu_rsync 
  
  ▶ 명령어 형태 : ssu_rsync src(소스코드 또는 디렉토리) dst(목적 디렉토리)

  ▶ 인자로 주어진 src 파일 또는 디렉토리를 dst디렉토리에 동기화한다.

  ▶ 동기화 시 src 파일이 dst 디렉토리 내 동일한 파일로 존재할 경우 해당 파일은 동기화 하지 않는다.

  ▶ 동기화 작업 중 SIGINT가 발생하면 동기화 작업이 취소되고 dst 디렉토리 내 파일들이 동기화하지 않은 상태로 유지되어야 한다.

  ▶ 이때, 동기화 중에는 사용자가 동기화 중인 파일을 open하는 것을 허용하지 않는다.

  ▶ src 파일 dst 디렉토리 내 파일의 동기화가 모두 완료되면 “ssu_rsync_log” 로그파일에 기록한다.
  
- 설계
  
  - main_crontab.c
    
    ![6](https://user-images.githubusercontent.com/47939832/112304108-71847080-8ce0-11eb-9fb1-900990346c06.png)
    
    - ssu_crontab
    
      ![7-1](https://user-images.githubusercontent.com/47939832/112304115-72b59d80-8ce0-11eb-9b3e-71bf69e2dacf.png)
    
      ![7-2](https://user-images.githubusercontent.com/47939832/112304122-747f6100-8ce0-11eb-93bb-f7f595a0da1b.png)
      
  - main_crond.c
    
    ![8](https://user-images.githubusercontent.com/47939832/112304126-75b08e00-8ce0-11eb-8cec-d881944d2385.png)
    
    - ssu_crond
      
      ![9](https://user-images.githubusercontent.com/47939832/112304129-75b08e00-8ce0-11eb-8db8-6265218ee183.png)
      
  - main_rsyn.c.
    
    ![10](https://user-images.githubusercontent.com/47939832/112304130-76492480-8ce0-11eb-8c57-fcf62575470b.png)
    
    - ssu_rsync.c
      
      ![11](https://user-images.githubusercontent.com/47939832/112304132-76e1bb00-8ce0-11eb-9b89-c19a7d0e5198.png)
      
- 구현
 
  <ssu_crontab.h>
  ~~~
  void ssu_crontab();// ssu_crontab_file을 읽어 명령 주기를 check, 시간 주기 입력이 잘못 되었다면 다시 입력 받을 수 있도록 해줌
  void command_add();// add 명령어를 수행한 후 ssu_crontab_file에 저장해줌
  void command_remove();// remove 명령어를 수행해주는 함수
  ~~~

  <ssu_crond.h>
  ~~~
  void ssu_crond();// 디몬 프로그램을 실행 시켜주는 함수
  int exec_crond();// 실질적으로 ssu_crond()함수를 실행시켜 주는 함수
  char *compare_command_remove(int prev, int pres);// 어떤 명령어가 삭제되었는지 check하여 ssu_crontab_log에 기록을 남겨주는 함수
  ~~~

  <ssu_rsync.h>
  ~~~
  void ssu_rsync(int argc, char *argv[]);// 입력 인자의 에러를 잡아주며, 접근 권한을 check해 주는 함수
  void exec_rsync();// 동기화 작업을 위해 src,dst의 절대경로, 상대경로를 check하여 프로그램을 실행하기에 알맞도록 변수 값의 호환을 유지시켜줌

  void option_rsync();// 동기화 작업을 실행하며, 옵션에 따라 다르게 동작함
  void rOption_rsync(char dir[PATH_MAX], FILE *fp);// r옵션이 실행되면 서브 디렉토리 또한 동기화 시켜줘야하므로 함수를 만들어 재귀적으로 동작해 서브 디렉토리 동기화 작업을 수행함
  ~~~
