import * as ts from 'typescript';
import * as fs from 'fs';
import * as path from 'path';
/*
import { analyzeAndGenerateTypes } from './type_analyzer';
import * as path from 'path';
// 현재 디렉토리를 루트 디렉토리로 사용
const rootDir = process.cwd();
const outputPath = path.join(rootDir, 'generated_types.ts');
console.log('TypeScript 타입 분석을 시작합니다...');
console.log(`루트 디렉토리: ${rootDir}`);
console.log(`출력 파일: ${outputPath}`);

try {
  analyzeAndGenerateTypes(rootDir, outputPath);
  console.log('타입 분석이 완료되었습니다.');
  console.log(`생성된 파일: ${outputPath}`);
} catch (error) {
  console.error('타입 분석 중 오류가 발생했습니다:', error);
  process.exit(1);
}
*/

interface TypeInfo {
  name: string;
  kind: 'interface' | 'type' | 'enum';
  content: string;
  dependencies: string[];
  filePath: string;
}

class TypeScriptAnalyzer {
  private types: Map<string, TypeInfo> = new Map();
  private program: ts.Program;
  private typeChecker: ts.TypeChecker;
  private sourceFiles: ts.SourceFile[];


  constructor(private rootDir: string) {
    // TypeScript 프로젝트 설정
    const tsConfigPath = ts.findConfigFile(rootDir, ts.sys.fileExists, 'tsconfig.json');
    const configFile = ts.readConfigFile(tsConfigPath || '', ts.sys.readFile);
    const parsedConfig = ts.parseJsonConfigFileContent(
      configFile.config,
      ts.sys,
      rootDir
    );

    // 프로그램 생성
    this.program = ts.createProgram(parsedConfig.fileNames, parsedConfig.options);
    this.typeChecker = this.program.getTypeChecker();
    this.sourceFiles = this.program.getSourceFiles().filter(file => 
      !file.isDeclarationFile && file.fileName.startsWith(rootDir)
    );
  }

  public analyze(): TypeInfo[] {
    this.sourceFiles.forEach(sourceFile => {
      this.visitNode(sourceFile);
    });

    return Array.from(this.types.values());
  }

  private visitNode(node: ts.Node): void {
    if (ts.isInterfaceDeclaration(node) || ts.isTypeAliasDeclaration(node) || ts.isEnumDeclaration(node)) {
      const typeInfo = this.extractTypeInfo(node);
      if (typeInfo) {
        this.types.set(typeInfo.name, typeInfo);
      }
    }

    ts.forEachChild(node, child => this.visitNode(child));
  }

  private extractTypeInfo(node: ts.Node): TypeInfo | null {
    if (!ts.isInterfaceDeclaration(node) && !ts.isTypeAliasDeclaration(node) && !ts.isEnumDeclaration(node)) {
      return null;
    }

    const name = (node as any).name.text;
    const kind = this.getNodeKind(node);
    const content = this.getNodeContent(node);
    const dependencies = this.extractDependencies(node);
    const filePath = (node as any).getSourceFile().fileName;

    return {
      name,
      kind,
      content,
      dependencies,
      filePath
    };
  }

  private getNodeKind(node: ts.Node): 'interface' | 'type' | 'enum' {
    if (ts.isInterfaceDeclaration(node)) return 'interface';
    if (ts.isTypeAliasDeclaration(node)) return 'type';
    if (ts.isEnumDeclaration(node)) return 'enum';
    return 'type';
  }

  private getNodeContent(node: ts.Node): string {
    const sourceFile = node.getSourceFile();
    const start = node.getStart(sourceFile);
    const end = node.getEnd();
    return sourceFile.text.substring(start, end);
  }

  private extractDependencies(node: ts.Node): string[] {
    const dependencies: string[] = [];
    
    const visit = (node: ts.Node) => {
      if (ts.isTypeReferenceNode(node)) {
        const typeName = node.typeName.getText();
        if (!dependencies.includes(typeName)) {
          dependencies.push(typeName);
        }
      }
      ts.forEachChild(node, visit);
    };

    visit(node);
    return dependencies;
  }
}

class TypeScriptGenerator {
  constructor(private types: TypeInfo[]) {}

  public generateFile(outputPath: string): void {
    const content = this.generateContent();
    fs.writeFileSync(outputPath, content, 'utf8');
  }

  private generateContent(): string {
    const imports = this.generateImports();
    const typeDefinitions = this.generateTypeDefinitions();
    const exports = this.generateExports();

    return `${imports}

${typeDefinitions}

${exports}`;
  }

  private generateImports(): string {
    const imports = new Set<string>();
    
    this.types.forEach(type => {
      type.dependencies.forEach(dep => {
        if (this.types.some(t => t.name === dep)) {
          imports.add(`import { ${dep} } from './${path.basename(type.filePath, '.ts')}';`);
        }
      });
    });

    return Array.from(imports).join('\n');
  }

  private generateTypeDefinitions(): string {
    return this.types.map(type => type.content).join('\n\n');
  }

  private generateExports(): string {
    const typeNames = this.types.map(type => type.name);
    return `export {\n  ${typeNames.join(',\n  ')}\n};`;
  }
}

// 사용 예시
function analyzeAndGenerateTypes(rootDir: string, outputPath: string): void {
  const analyzer = new TypeScriptAnalyzer(rootDir);
  const types = analyzer.analyze();
  const generator = new TypeScriptGenerator(types);
  generator.generateFile(outputPath);
}

// CLI에서 실행할 수 있도록 export
export { analyzeAndGenerateTypes, TypeScriptAnalyzer, TypeScriptGenerator }; 

/*[example]
// 기본 인터페이스
interface User {
  id: number;
  name: string;
  email: string;
  age?: number;
}

// 타입 별칭
type UserRole = 'admin' | 'user' | 'guest';

// 열거형
enum Status {
  Active = 'ACTIVE',
  Inactive = 'INACTIVE',
  Pending = 'PENDING'
}

// 중첩된 인터페이스
interface Address {
  street: string;
  city: string;
  country: string;
  zipCode: string;
}

interface UserProfile extends User {
  address: Address;
  role: UserRole;
  status: Status;
  createdAt: Date;
  updatedAt: Date;
}

// 제네릭 타입
interface Response<T> {
  data: T;
  status: number;
  message: string;
}

// 유니온 타입
type ID = string | number;

// 인터섹션 타입
type AdminUser = User & {
  permissions: string[];
  isSuperAdmin: boolean;
};

// 함수 타입
type UserCallback = (user: User) => void;

// 매핑된 타입
type ReadonlyUser = {
  readonly [K in keyof User]: User[K];
};

// 조건부 타입
type NonNullableUser = {
  [K in keyof User]: User[K] extends null | undefined ? never : User[K];
}; 
*/
