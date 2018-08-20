namespace nex::util
{
	/**
	 * Returns a pointer that is (positively) shifted by a specific number of bytes from a given pointer.
	 */
	inline void* add(void* pointer, int byteNumber)
	{
		return reinterpret_cast<void*>(reinterpret_cast<char*>(pointer) + byteNumber);
	}

	/**
	* Returns a pointer that is (negatively) shifted by a specific number of bytes from a given pointer.
	*/
	inline void* subtract(void* pointer, int byteNumber)
	{
		return reinterpret_cast<void*>(reinterpret_cast<char*>(pointer) - byteNumber);
	}
}