export function Logo({ className }: { className?: string }) {
  return (
    <svg viewBox="0 0 32 32" className={className} fill="none" aria-hidden>
      <circle cx="16" cy="16" r="15.5" fill="currentColor" />
      <circle cx="16" cy="16" r="10" className="stroke-paper" strokeOpacity="0.25" />
      <circle cx="16" cy="16" r="6.5" className="stroke-paper" strokeOpacity="0.25" />
      <circle cx="16" cy="16" r="3.4" className="fill-paper" />
      <circle cx="16" cy="16" r="0.9" fill="currentColor" />
    </svg>
  );
}
