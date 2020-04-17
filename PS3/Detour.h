#pragma once
//IDK why but this fixes intellisense issues for some weird reason
DWORD nullShit;

int Int24ToInt32(int Value) {
	byte bValue = (Value >> 24) & 0xFF;
	Value &= 0x00FFFFFF;

	// if the value is supposed to be negative and is signed
	if (Value & 0x800000 && bValue != 0x48)
		Value |= 0xFF000000;

	// if the value has an extra one from 'b' calls then remove it
	if (Value & 1)
		Value -= 1;
	return Value;
}

int GetBranchCall(int Address)
{
	int Dest, Temp;
	Dest = *(int *)Address;
	Temp = Dest;
	Dest = Temp & 0x03FFFFFC;
	if (Temp & 0x02000000) Dest |= 0xFC000000;
	Dest = Address + Dest;
	return Dest;
}

void PatchInJump(DWORD* Address, void* Dest, BOOL Linked)
{
	DWORD FuncBytes[4];													// Use this data to copy over the address.
	DWORD Destination = (DWORD)Dest;

	FuncBytes[0] = 0x3D600000 + ((Destination >> 16) & 0xFFFF);			// lis %r11, dest>>16
	if (Destination & 0x8000)											// if bit 16 is 1...
		FuncBytes[0] += 1;
	FuncBytes[1] = 0x396B0000 + (Destination & 0xFFFF);					// addi %r11, %r11, dest&OxFFFF
	FuncBytes[2] = 0x7D6903A6;											// mtctr %r11
	FuncBytes[3] = 0x4E800420;											// bctr
	if (Linked)
		FuncBytes[3] += 1;												// bctrl

	write_process((void*)Address, FuncBytes, 4 * 4);
}

struct Detour
{
	opd_s opd;
	BYTE OrigAsm[0x10];
	DWORD OrigAddr;
	DWORD StubAddr;
	DWORD DestAddr;
	opd_s* StubCall;
	BOOL SetupDetour(DWORD Address, DWORD ExecuteableTOC, PVOID Destination, PVOID Stub);
	void TakeDownDetour();
	template<typename T>
	T CallOriginal();
	template<typename T, typename R3>
	T CallOriginal(R3 r3);
	template<typename T, typename R3, typename R4>
	T CallOriginal(R3 r3, R4 r4);
	template<typename T, typename R3, typename R4, typename R5>
	T CallOriginal(R3 r3, R4 r4, R5 r5);
	template<typename T, typename R3, typename R4, typename R5, typename R6>
	T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7>
	T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8>
	T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9>
	T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10>
	T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1, typename S2>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1, typename S2, typename S3>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
			S3 s3);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1, typename S2, typename S3, typename S4>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
			S3 s3, S4 s4);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1, typename S2, typename S3, typename S4, typename S5>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
			S3 s3, S4 s4, S5 s5);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1, typename S2, typename S3, typename S4, typename S5, typename S6>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
			S3 s3, S4 s4, S5 s5, S6 s6);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
			S3 s3, S4 s4, S5 s5, S6 s6, S7 s7);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
			S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
		typename S9>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
			S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
		typename S9, typename S10>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
			S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
		typename S9, typename S10, typename S11>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
			S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
		typename S9, typename S10, typename S11, typename S12>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
			S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11, S12 s12);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
		typename S9, typename S10, typename S11, typename S12, typename S13>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
			S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11, S12 s12, S13 s13);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
		typename S9, typename S10, typename S11, typename S12, typename S13, typename S14>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
			S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11, S12 s12, S13 s13, S14 s14);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
		typename S9, typename S10, typename S11, typename S12, typename S13, typename S14, typename S15>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
			S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11, S12 s12, S13 s13, S14 s14,
			S15 s15);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
		typename S9, typename S10, typename S11, typename S12, typename S13, typename S14, typename S15, typename S16>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
			S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11, S12 s12, S13 s13, S14 s14,
			S15 s15, S16 s16);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
		typename S9, typename S10, typename S11, typename S12, typename S13, typename S14, typename S15, typename S16,
		typename S17>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
			S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11, S12 s12, S13 s13, S14 s14,
			S15 s15, S16 s16, S17 s17);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
		typename S9, typename S10, typename S11, typename S12, typename S13, typename S14, typename S15, typename S16,
		typename S17, typename S18>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
			S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11, S12 s12, S13 s13, S14 s14,
			S15 s15, S16 s16, S17 s17, S18 s18);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
		typename S9, typename S10, typename S11, typename S12, typename S13, typename S14, typename S15, typename S16,
		typename S17, typename S18, typename S19>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
			S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11, S12 s12, S13 s13, S14 s14,
			S15 s15, S16 s16, S17 s17, S18 s18, S19 s19);
	template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
		typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
		typename S9, typename S10, typename S11, typename S12, typename S13, typename S14, typename S15, typename S16,
		typename S17, typename S18, typename S19, typename S20>
		T CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
			S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11, S12 s12, S13 s13, S14 s14,
			S15 s15, S16 s16, S17 s17, S18 s18, S19 s19, S20 s20);
};

///Will hook the address specified
BOOL Detour::SetupDetour(DWORD Address, DWORD ExecuteableTOC, PVOID Destination, PVOID Stub)
{
	if (this->OrigAddr != Address || this->StubAddr == 0) {

		this->StubAddr = *(DWORD*)Stub;
		this->DestAddr = *(DWORD*)Destination;
		this->opd.sub = this->StubAddr;
		this->opd.toc = ExecuteableTOC;
		this->StubCall = &this->opd;

		// save the address incase we take-down the detour
		this->OrigAddr = Address;
		// Copy the asm bytes before we replace it with the hook
		write_process((PVOID)this->StubAddr, (PVOID)Address, 0x10);

		// make the stub call the orig function
		PatchInJump((PDWORD)(this->StubAddr + 0x10), (PVOID)(this->OrigAddr + 0x10), FALSE);
		printf("Detour Stub Jump Location 0x%X\n", this->OrigAddr + 0x10);

		// apply the hook in the function
		PatchInJump((PDWORD)this->OrigAddr, (PVOID)this->DestAddr, FALSE);
		printf("Detour Function Hook Jump Location 0x%X\n", this->DestAddr);
	}
	return TRUE;
}

void Detour::TakeDownDetour()
{
	if (this->OrigAddr)
		write_process((PVOID)this->OrigAddr, (PVOID)this->StubAddr, 0x10);
}

template<typename T>
T Detour::CallOriginal()
{
	return ((T(*)(...))this->StubCall)();
}

template<typename T, typename R3>
T Detour::CallOriginal(R3 r3)
{
	return ((T(*)(...))this->StubCall)(r3);
}

template<typename T, typename R3, typename R4>
T Detour::CallOriginal(R3 r3, R4 r4)
{
	return ((T(*)(...))this->StubCall)(r3, r4);
}

template<typename T, typename R3, typename R4, typename R5>
T Detour::CallOriginal(R3 r3, R4 r4, R5 r5)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5);
}

template<typename T, typename R3, typename R4, typename R5, typename R6>
T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7>
T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8>
T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9>
T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10>
T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1, typename S2>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1, s2);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1, typename S2, typename S3>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
		S3 s3)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1, s2, s3);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1, typename S2, typename S3, typename S4>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
		S3 s3, S4 s4)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1, s2, s3, s4);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1, typename S2, typename S3, typename S4, typename S5>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
		S3 s3, S4 s4, S5 s5)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1, s2, s3, s4, s5);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1, typename S2, typename S3, typename S4, typename S5, typename S6>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
		S3 s3, S4 s4, S5 s5, S6 s6)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1, s2, s3, s4, s5, s6);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
		S3 s3, S4 s4, S5 s5, S6 s6, S7 s7)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1, s2, s3, s4, s5, s6, s7);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
		S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1, s2, s3, s4, s5, s6, s7, s8);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
	typename S9>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
		S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1, s2, s3, s4, s5, s6, s7, s8, s9);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
	typename S9, typename S10>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
		S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
	typename S9, typename S10, typename S11>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
		S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10,
		s11);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
	typename S9, typename S10, typename S11, typename S12>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
		S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11, S12 s12)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10,
		s11, s12);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
	typename S9, typename S10, typename S11, typename S12, typename S13>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
		S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11, S12 s12, S13 s13)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10,
		s11, s12, s13);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
	typename S9, typename S10, typename S11, typename S12, typename S13, typename S14>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
		S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11, S12 s12, S13 s13, S14 s14)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10,
		s11, s12, s13, s14);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
	typename S9, typename S10, typename S11, typename S12, typename S13, typename S14, typename S15>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
		S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11, S12 s12, S13 s13, S14 s14,
		S15 s15)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10,
		s11, s12, s13, s14, s15);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
	typename S9, typename S10, typename S11, typename S12, typename S13, typename S14, typename S15, typename S16>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
		S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11, S12 s12, S13 s13, S14 s14,
		S15 s15, S16 s16)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10,
		s11, s12, s13, s14, s15, s16);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
	typename S9, typename S10, typename S11, typename S12, typename S13, typename S14, typename S15, typename S16,
	typename S17>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
		S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11, S12 s12, S13 s13, S14 s14,
		S15 s15, S16 s16, S17 s17)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10,
		s11, s12, s13, s14, s15, s16, s17);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
	typename S9, typename S10, typename S11, typename S12, typename S13, typename S14, typename S15, typename S16,
	typename S17, typename S18>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
		S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11, S12 s12, S13 s13, S14 s14,
		S15 s15, S16 s16, S17 s17, S18 s18)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10,
		s11, s12, s13, s14, s15, s16, s17, s18);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
	typename S9, typename S10, typename S11, typename S12, typename S13, typename S14, typename S15, typename S16,
	typename S17, typename S18, typename S19>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
		S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11, S12 s12, S13 s13, S14 s14,
		S15 s15, S16 s16, S17 s17, S18 s18, S19 s19)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10,
		s11, s12, s13, s14, s15, s16, s17, s18, s19);
}

template<typename T, typename R3, typename R4, typename R5, typename R6, typename R7, typename R8, typename R9, typename R10,
	typename S1, typename S2, typename S3, typename S4, typename S5, typename S6, typename S7, typename S8,
	typename S9, typename S10, typename S11, typename S12, typename S13, typename S14, typename S15, typename S16,
	typename S17, typename S18, typename S19, typename S20>
	T Detour::CallOriginal(R3 r3, R4 r4, R5 r5, R6 r6, R7 r7, R8 r8, R9 r9, R10 r10, S1 s1, S2 s2,
		S3 s3, S4 s4, S5 s5, S6 s6, S7 s7, S8 s8, S9 s9, S10 s10, S11 s11, S12 s12, S13 s13, S14 s14,
		S15 s15, S16 s16, S17 s17, S18 s18, S19 s19, S20 s20)
{
	return ((T(*)(...))this->StubCall)(r3, r4, r5, r6, r7, r8, r9, r10, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10,
		s11, s12, s13, s14, s15, s16, s17, s18, s19, s20);
};
