# Pipeline código fonte -> executável

Baseado nos arquivos deste diretório: `demo.c`, `sum.h`, `demo.pre.c`, `demo.asm`, `demo.o`, `demo`, `a.out`.

Fonte usado:

```c
// demo.c
#include "sum.h"

void __exit(void)
{
    __asm__(
        "     mov rax, 60\n"
    );
}

int main(void)
{
  int a = 10;
  int b = 20;
  int c = sum(a, b);
  __exit();
  return 0;
}

int _start(void)
{
  return main();
}
```

```c
// sum.h
int sum(int a, int b)
{
  return a + b;
}
```

---

## 1. Preprocessamento (cpp)

Comando: `gcc -E demo.c -o demo.pre.c`

O que faz: expande `#include`, `#define`, macros, remove comentários, resolve diretivas condicionais (`#ifdef` etc). Produz um único arquivo `.c` só com C puro, sem diretivas.

Saída real (`demo.pre.c`): `#include "sum.h"` foi substituído pelo conteúdo literal do header. Linhas `# N "arquivo"` são marcadores de linha pro compilador saber de qual arquivo original cada trecho veio (importante pra mensagens de erro e debug).

```c
# 1 "sum.h" 1
int sum(int a, int b)
{
  return a + b;
}
# 2 "demo.c" 2

void __exit(void)
{
    __asm__(
        "     mov rax, 60\n"
  );
}
```

Ainda não é compilado. É texto C puro.

## 2. Compilação (cc1 / análise + geração de assembly)

Comando: `gcc -S demo.pre.c -o demo.asm`

Etapas internas do compilador propriamente dito:

- **Análise léxica**: texto vira tokens (`int`, `sum`, `(`, `a`, ...).
- **Análise sintática**: tokens viram AST (árvore de sintaxe), valida gramática C.
- **Análise semântica**: checa tipos, escopos, declarações. `sum(a, b)` casado com protótipo em `sum.h`.
- **Otimização** (se `-O` usado; aqui `-O0` implícito, sem otimizar).
- **Geração de código**: AST vira assembly para arquitetura alvo (aqui x86-64, sintaxe Intel).

Saída real (`demo.asm`), trecho da função `sum`:

```asm
sum:
	push	rbp
	mov	rbp, rsp
	mov	DWORD PTR -4[rbp], edi
	mov	DWORD PTR -8[rbp], esi
	mov	edx, DWORD PTR -4[rbp]
	mov	eax, DWORD PTR -8[rbp]
	add	eax, edx
	pop	rbp
	ret
```

Note bloco `__asm__("mov rax, 60")` do `demo.c` foi inserido literal dentro de `__exit`, marcado com `#APP` / `#NO_APP` — assembly inline passa direto, sem tradução.

Ainda é texto legível, não binário.

## 3. Assembly (as)

Comando: `as demo.asm -o demo.o` (ou `gcc -c`)

O assembler traduz instruções mnemônicas (`mov`, `add`, `call`) em opcodes binários (código de máquina), e monta isso em formato objeto **ELF relocável** (`.o`).

Características do `.o` gerado:

```
demo.o: ELF 64-bit LSB relocatable, x86-64, version 1 (SYSV), not stripped
```

Tabela de símbolos (via `nm demo.o`):

```
0000000000000018 T __exit
000000000000002a T main
0000000000000062 T _start
0000000000000000 T sum
```

`T` = símbolo definido na seção `.text` (código). Nesse ponto `call sum` dentro de `main` ainda não tem endereço final — é uma **referência não resolvida**, resolvida só no link. `.o` sozinho não executa.

## 4. Linkagem (ld)

Comando: `gcc demo.o -o demo` (chama `ld` por trás) ou `gcc demo.o -o a.out -no-pie` / variantes.

O linker:

1. Junta um ou mais `.o` (aqui só um, mas normalmente múltiplos arquivos + bibliotecas).
2. Resolve referências entre símbolos: cada `call sum`, `call main` recebe endereço final.
3. Linka bibliotecas (estáticas `.a` ou dinâmicas `.so`). Aqui `demo` saiu **estaticamente linkado**, `a.out` saiu **dinamicamente linkado** (depende de `/lib64/ld-linux-x86-64.so.2` em runtime).
4. Monta layout final de memória: seções `.text` (código), `.data`, `.rodata`, `.bss`, define entry point.
5. Produz executável ELF completo.

Diferença observada:

```
demo:   ELF 64-bit LSB executable, x86-64, statically linked, not stripped
a.out:  ELF 64-bit LSB pie executable, x86-64, dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2
```

`demo` não depende de libs externas em runtime. `a.out` é PIE (Position Independent Executable, ASLR-friendly) e carrega `ld-linux` pra resolver libs dinâmicas (libc etc) na hora de rodar.

## 5. Execução

Kernel carrega ELF, mapeia seções em memória, resolve loader dinâmico se aplicável (caso `a.out`), e salta pro entry point (aqui `_start`, que esse código define manualmente chamando `main`).

---

## Resumo do pipeline

```
demo.c  --[preprocessador: cpp]-->       demo.pre.c   (C puro, macros expandidas)
demo.pre.c --[compilador: cc1]-->        demo.asm     (assembly textual)
demo.asm --[assembler: as]-->            demo.o       (código de máquina, ELF relocável, símbolos não resolvidos)
demo.o  --[linker: ld]-->                demo / a.out (ELF executável, símbolos resolvidos, pronto pra rodar)
```

Cada seta é um programa separado — `gcc` é só um orquestrador que chama `cpp`, `cc1`, `as`, `ld` em sequência (ver `gcc -v demo.c -o demo` pra ver os comandos reais invocados).

Bônus — `sum.h.gch`: header pré-compilado (precompiled header). Arquivo binário que serializa resultado do parse/semantic de `sum.h` pra pular reprocessamento em compilações futuras. Usado via `#include` quando `.gch` existe ao lado do `.h` com nome igual.
