# RHIDX12 Review Focus

When reviewing RHIDX12 code:
- Ensure proper use of COM interfaces and reference counting; if not, watch for memory leaks — every `Create*` should have a corresponding `Release`.
- Make sure that any functions returning HRESULT are checked for with CheckD3DResult(), if not handled explicitly.
- Verify synchronization (resource barriers, UAV barriers, etc.).
- Check for correct D3D12_\* compatibility with core RHI types.
