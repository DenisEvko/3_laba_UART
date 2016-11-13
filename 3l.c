#include "stm32f10x.h"
#include "stdbool.h"
#include "string.h"

char RxBuff[256];
bool ComReceived;
void Init_UART(void);

//обработка прерывания (сохранение принятых данных)
void USART1_IRQHandler(void){
	char tmp;

	if ((USART1->SR & USART_SR_RXNE)>0){ //если строка не пустая
		tmp = USART1->DR;
		if (tmp != 0x0d){
			RxBuff[strlen(RxBuff)] = tmp;
		}
		else ComReceived = true;	//флаг приема (если происходит прием)
	}
}

//настройка USART
void Init_UART(void){
	//тактирование
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	
	//PA9 'OUT' (ALTERNATIVE FUNC PUSH-PULL)
	GPIOA->CRH &= ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9);
	GPIOA->CRH |= (GPIO_CRH_CNF9_1 | GPIO_CRH_MODE9_1);
	//PA10 'IN' DEFAULT
	
	USART1->BRR=0x4E2; //настройка частоты формула из даташита
	USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;  // включить приемник и передатчик
	USART1->CR1 |= USART_CR1_RXNEIE;	//RXNE Interrupt Enable
	
	NVIC_EnableIRQ(USART1_IRQn);	//разрешение прерываний
}

//отправка
void TxString(char *str){
	uint8_t i;
	int j;
	
	for(j=0;j<=100000;j++){}
		strcat(str, "\n"); //запись символа конца в конец отправляемой строки
	
		for (i=0; i < strlen(str); i++){
		while ((USART1->SR & USART_SR_TC) == 0) {};
		USART1->DR = str[i];
	}
}

void ExecuteCommand(void){
	TxString(RxBuff);
	
	memset(RxBuff, 0, sizeof(RxBuff));//очистка буфера
	ComReceived=false; //флаг  
}

int main(void){

	Init_UART();

	while(true){
		if (ComReceived) ExecuteCommand();
	}
}
