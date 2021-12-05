# linpack_test
Intel Linpack을 위해 만들어진 frontend입니다.

## 준비사항
1. https://www.intel.com/content/www/us/en/developer/articles/technical/onemkl-benchmarks-suite.html
에서 Windows 패키지를 다운로드 합니다.

2. 다운로드 받은 압축 파일에서
- `windows\mkl\benchmarks\linpack\linpack_xeon64.exe`
- `windows\compiler\intel64\libiomp5md.dll`

두 파일을 `library` 폴더로 옮겨줍니다. 

3. `linpackTest.exe`파일로 프로그램을 실행할 수 있습니다. 

## 사용 방법
- Mode 1: Test All Core at Same Time

CPU의 모든 코어와 스레드를 이용하여, linpack benchmark를 수행합니다.

- Mode 2: Test Thread Usage

지정한 스레드 만큼을 이용하여, linpack benchmark를 수행합니다. 

- Mode 3: Test All Core One by One

CPU의 모든 코어를, 돌아가면서 하나씩 테스트 합니다. 

- Mode 4: Test Specific Cores

각각의 개별 코어를 테스트 합니다. 어떤 코어를 테스트 할지 직접 지정할 수 있습니다. 
