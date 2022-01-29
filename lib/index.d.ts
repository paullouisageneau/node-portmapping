export as namespace nodePortMapping;

export type LogLevel = 'Verbose'|'Debug'|'Info'|'Warning'|'Error'|'Fatal';

export interface Config {
    logLevel?: LogLevel;
}

export function init(config?: Config): void;
export function cleanup(): Promise<void>;

export type State = 'Pending'|'Success'|'Failure'|'Destroying'|'Destroyed';
export type Protocol = 'TCP'|'UDP';

export interface MappingSpec {
    internalPort: number;
    externalPort?: number;
    protocol?: Protocol;
}

export interface MappingInfo {
    state: State;
    internalPort: number;
    externalPort: number;
    externalHost: string;
    protocol: Protocol;
}

export type MappingCallback = (info: MappingInfo) => {};

export interface Mapping {
    destroy(): void;
    getInfo(): MappingInfo;
}

export function createMapping(spec: MappingSpec|number, callback?: MappingCallback): Mapping;

export function getLocalAddress(): string;

