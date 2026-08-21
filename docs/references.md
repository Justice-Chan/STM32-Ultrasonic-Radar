# References

This repository does not include local copies of vendor documentation. The
following links point to the official STMicroelectronics sources so readers can
check the latest available revisions.

| Document | Purpose | Usage in This Project |
| --- | --- | --- |
| [STM32F411 Documentation](https://www.st.com/en/microcontrollers-microprocessors/stm32f411/documentation.html) | Official documentation index | Locating current datasheets, reference manuals, and errata |
| [DS10314 - STM32F411xC/E Datasheet](https://www.st.com/resource/en/datasheet/stm32f411ce.pdf) | Device pinout, alternate functions, package, and electrical limits | Verifying PA2/PA3 AF7, PA4, PA5, PB6, and GPIO voltage constraints |
| [RM0383 - STM32F411 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0383-stm32f411xce-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) | Memory map and peripheral register behavior | RCC, GPIO, SYSCFG/EXTI, DMA1, TIM2, TIM4, and USART2 configuration |
| [PM0214 - STM32 Cortex-M4 Programming Manual](https://www.st.com/resource/en/programming_manual/dm00046982-stm32-cortex-m4-mcus-and-mpus-programming-manual-stmicroelectronics.pdf) | Cortex-M4 core architecture and core peripherals | NVIC, interrupt handling, exception behavior, and the vector table |

## Choosing the Right Document

- Use the **datasheet** to answer: Functions pins can perform and Electrical limits.
- Use the **reference manual** to answer: Peripheral registers and bit control behavior.
- Use the **programming manual** for: CPU-core topics such as NVIC, exceptions,
  and interrupt execution.

Document revisions may change. Check the official STM32F411 documentation page
before relying on a specific revision.